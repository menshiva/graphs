#include "ToolCreator.h"
#include "ToolCreatorPanelWidget.h"
#include "Graphs/EntityStorage/Commands/EdgeCommands.h"
#include "Graphs/EntityStorage/Commands/GraphCommands.h"
#include "Graphs/EntityStorage/Commands/VertexCommands.h"

DECLARE_CYCLE_STAT(TEXT("UToolCreator::TickTool"), STAT_UToolCreator_TickTool, STATGROUP_GRAPHS_PERF);
DECLARE_CYCLE_STAT(TEXT("UToolCreator::OnRightTriggerAction"), STAT_UToolCreator_OnRightTriggerAction, STATGROUP_GRAPHS_PERF);

UToolCreator::UToolCreator(const FObjectInitializer &ObjectInitializer) : UTool(
	"Create",
	TEXT("/Game/Graphs/UI/Icons/Add"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolCreatorPanel")
) {
	const ConstructorHelpers::FObjectFinder<UMaterial> PreviewMaterialAsset(
		TEXT("/Game/Graphs/Materials/TeleportPreviewMaterial")
	);

	const ConstructorHelpers::FObjectFinder<UStaticMesh> SpherePreviewMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
	VertexPreviewMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(
		this, "ToolCreatorSpherePreviewMesh"
	);
	VertexPreviewMesh->SetStaticMesh(SpherePreviewMeshAsset.Object);
	VertexPreviewMesh->SetVisibility(false);
	VertexPreviewMesh->SetCollisionProfileName(TEXT("NoCollision"));
	VertexPreviewMesh->SetCastShadow(false);
	VertexPreviewMesh->SetRelativeScale3D(FVector(UVerticesRenderer::MeshScale / 50.0f));
	const auto SpherePreviewMaterialInst = VertexPreviewMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(
		0,
		PreviewMaterialAsset.Object
	);
	SpherePreviewMaterialInst->SetVectorParameterValue("Color", ColorConsts::GreenColor.ReinterpretAsLinear());
	VertexPreviewMesh->SetMaterial(0, SpherePreviewMaterialInst);
	VertexPreviewMesh->SetupAttachment(this);

	const ConstructorHelpers::FObjectFinder<UStaticMesh> EdgePreviewMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	EdgePreviewMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(
		this, "ToolCreatorEdgePreviewMesh"
	);
	EdgePreviewMesh->SetStaticMesh(EdgePreviewMeshAsset.Object);
	EdgePreviewMesh->SetVisibility(false);
	EdgePreviewMesh->SetCollisionProfileName(TEXT("NoCollision"));
	EdgePreviewMesh->SetCastShadow(false);
	EdgePreviewMesh->SetRelativeScale3D(FVector(UEdgesRenderer::MeshScale / 100.0f));
	const auto EdgePreviewMaterialInst = EdgePreviewMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(
		0,
		PreviewMaterialAsset.Object
	);
	EdgePreviewMaterialInst->SetVectorParameterValue("Color", ColorConsts::GreenColor.ReinterpretAsLinear());
	EdgePreviewMesh->SetMaterial(0, EdgePreviewMaterialInst);
	EdgePreviewMesh->SetupAttachment(this);
}

void UToolCreator::SetMode(const CreationMode NewMode) {
	SetGraphSelection(EntityId::NONE());
	SetVertexSelection(EntityId::NONE());
	Mode = NewMode;

	switch (Mode) {
		case CreationMode::VERTEX: {
			SetSupportedEntities({GRAPH});
			break;
		}
		case CreationMode::EDGE: {
			SetSupportedEntities({VERTEX});
			break;
		}
		case CreationMode::GRAPH: {
			SetSupportedEntities({});
			break;
		}
		default: {
			check(false);
		}
	}
}

void UToolCreator::SetGraphSelection(const EntityId GraphId) {
	if (SelectedGraphId == GraphId)
		return;

	if (GraphId != EntityId::NONE()) {
		GraphCommands::Mutable::SetHit(GraphId, false);
		GraphCommands::Mutable::SetOverrideColor(GraphId, ColorConsts::GreenColor);
		GetGraphsRenderers()->MarkGraphDirty(
			GraphId,
			true, false,
			true, false
		);
		GetGraphsRenderers()->RedrawGraphChunksIfDirty(GraphId);
	}
	else {
		check(SelectedGraphId != EntityId::NONE());
		GraphCommands::Mutable::SetOverrideColor(SelectedGraphId, ColorConsts::OverrideColorNone);
		GetGraphsRenderers()->MarkGraphDirty(
			SelectedGraphId,
			true, false,
			true, false
		);
		GetGraphsRenderers()->RedrawGraphChunksIfDirty(SelectedGraphId);
	}

	SelectedGraphId = GraphId;
	GetToolPanel<UToolCreatorPanelWidget>()->Update();
}

void UToolCreator::SetVertexSelection(const EntityId VertexId) {
	if (SelectedVertexId == VertexId)
		return;

	if (VertexId != EntityId::NONE()) {
		VertexCommands::Mutable::SetHit(VertexId, false);
		VertexCommands::Mutable::SetOverrideColor(VertexId, ColorConsts::GreenColor);
		GetGraphsRenderers()->MarkVertexDirty(VertexId, true, false, false);
		GetGraphsRenderers()->RedrawChunkByVertexIfDirty(VertexId, false);
	}
	else {
		check(SelectedVertexId != EntityId::NONE());
		VertexCommands::Mutable::SetOverrideColor(SelectedVertexId, ColorConsts::OverrideColorNone);
		GetGraphsRenderers()->MarkVertexDirty(SelectedVertexId, true, false, false);
		GetGraphsRenderers()->RedrawChunkByVertexIfDirty(SelectedVertexId, false);
	}

	SelectedVertexId = VertexId;
	GetToolPanel<UToolCreatorPanelWidget>()->Update();
}

void UToolCreator::OnAttach() {
	Super::OnAttach();
	GetVrRightController()->SetLaserActive(true);
}

void UToolCreator::OnDetach() {
	Super::OnDetach();

	VertexPreviewMesh->SetVisibility(false);
	EdgePreviewMesh->SetVisibility(false);
	GetVrRightController()->SetLaserColor(ColorConsts::BlueColor.ReinterpretAsLinear());
	GetVrRightController()->SetLaserLength(0.0f);
	GetVrRightController()->SetCastEnabled(true);
	GetVrRightController()->SetLaserActive(false);

	SetGraphSelection(EntityId::NONE());
}

void UToolCreator::TickTool() {
	SCOPE_CYCLE_COUNTER(STAT_UToolCreator_TickTool);
	Super::TickTool();

	// vertex preview
	{
		if (CheckVertexPreviewValidity()) {
			if (!VertexPreviewMesh->IsVisible()) {
				GetVrRightController()->SetCastEnabled(false);
				GetVrRightController()->SetLaserColor(ColorConsts::GreenColor.ReinterpretAsLinear());
				GetVrRightController()->SetLaserLength(DefaultPreviewDistance);
				VertexPreviewMesh->SetVisibility(true);
			}
		}
		else if (VertexPreviewMesh->IsVisible()) {
			GetVrRightController()->SetCastEnabled(true);
			VertexPreviewMesh->SetVisibility(false);
			GetVrRightController()->SetLaserColor(ColorConsts::BlueColor.ReinterpretAsLinear());
		}

		if (VertexPreviewMesh->IsVisible())
			VertexPreviewMesh->SetWorldLocation(GetVrRightController()->GetLaserEndPosition());
	}

	// edge preview
	{
		if (CheckEdgePreviewValidity()) {
			static EntityId PrevHitId = EntityId::NONE();

			if (!EdgePreviewMesh->IsVisible() || PrevHitId != GetHitEntityId()) {
				auto FirstVertexPos = ES::GetEntity<VertexEntity>(SelectedVertexId).Position;
				auto SecondVertexPos = ES::GetEntity<VertexEntity>(GetHitEntityId()).Position;

				// do not show edge preview if vertices have intersection
				if (FVector::DistSquared(FirstVertexPos, SecondVertexPos)
					> 4 * UVerticesRenderer::MeshScale * UVerticesRenderer::MeshScale)
				{
					const auto ForwardDir = (SecondVertexPos - FirstVertexPos).GetSafeNormal();

					// offset vertex positions almost by vertex mesh scale
					const auto Offset = ForwardDir * UVerticesRenderer::MeshScale * 0.95f;
					FirstVertexPos += Offset;
					SecondVertexPos -= Offset;

					EdgePreviewMesh->SetWorldLocationAndRotation(
						(FirstVertexPos + SecondVertexPos) / 2.0f,
						FQuat::FindBetweenNormals(FVector::UpVector, ForwardDir)
					);

					auto Scale = EdgePreviewMesh->GetComponentScale();
					Scale.Z = FVector::Dist(FirstVertexPos, SecondVertexPos) / 100.0f;
					EdgePreviewMesh->SetWorldScale3D(Scale);

					EdgePreviewMesh->SetVisibility(true);
				}
			}

			PrevHitId = GetHitEntityId();
		}
		else if (EdgePreviewMesh->IsVisible())
			EdgePreviewMesh->SetVisibility(false);
	}
}

bool UToolCreator::OnRightTriggerAction(const bool IsPressed) {
	if (!IsPressed)
		return Super::OnRightTriggerAction(IsPressed);

	SCOPE_CYCLE_COUNTER(STAT_UToolCreator_TickTool);

	if (Mode == CreationMode::VERTEX) {
		if (SelectedGraphId == EntityId::NONE()) {
			if (GetHitEntityId() != EntityId::NONE()) {
				GetVrRightController()->SetCastEnabled(false);
				SetGraphSelection(GetHitEntityId());
				return true;
			}
		}
		else {
			const auto VertexId = VertexCommands::Mutable::Create(
				SelectedGraphId,
				GraphCommands::Const::GenerateUniqueVertexUserId(SelectedGraphId),
				GetVrRightController()->GetLaserEndPosition(),
				ColorConsts::VertexDefaultColor,
				0
			);
			VertexCommands::Mutable::SetOverrideColor(VertexId, ColorConsts::GreenColor);
			GetGraphsRenderers()->AddVertexToChunk(VertexId);
			GetGraphsRenderers()->RedrawChunkByVertexIfDirty(VertexId, false);
			return true;
		}
	}
	else if (Mode == CreationMode::EDGE) {
		if (GetHitEntityId() != EntityId::NONE()) {
			if (SelectedVertexId == EntityId::NONE()) {
				SetVertexSelection(GetHitEntityId());
				return true;
			}
			if (CheckEdgePreviewValidity()) {
				const auto EdgeId = EdgeCommands::Mutable::Create(
					ES::GetEntity<VertexEntity>(SelectedVertexId).GraphId,
					SelectedVertexId, GetHitEntityId()
				);
				SetVertexSelection(EntityId::NONE());
				GetGraphsRenderers()->AddEdgeToChunk(EdgeId);
				GetGraphsRenderers()->RedrawChunkByEdgeIfDirty(EdgeId, false);
				return true;
			}
		}
	}
	else {
		check(Mode == CreationMode::GRAPH);
		const auto GraphId = GraphCommands::Mutable::Create(true);
		VertexCommands::Mutable::Create(
			GraphId,
			GraphCommands::Const::GenerateUniqueVertexUserId(GraphId),
			GetVrRightController()->GetLaserEndPosition(),
			ColorConsts::VertexDefaultColor,
			0
		);
		GetGraphsRenderers()->ConstructGraphChunks(GraphId);
		GetToolPanel<UToolCreatorPanelWidget>()->UpdateModeViaSelector(CreationMode::VERTEX);
		SetGraphSelection(GraphId);
		return true;
	}

	return Super::OnRightTriggerAction(IsPressed);
}

bool UToolCreator::OnRightThumbstickY(const float Value) {
	if (CheckVertexPreviewValidity()) {
		GetVrRightController()->SetLaserLengthDelta(Value);
		return true;
	}
	return Super::OnRightThumbstickY(Value);
}

bool UToolCreator::CheckVertexPreviewValidity() const {
	return !GetVrRightController()->IsInUiState()
		&& (Mode == CreationMode::GRAPH
			|| (Mode == CreationMode::VERTEX && SelectedGraphId != EntityId::NONE()));
}

bool UToolCreator::CheckEdgePreviewValidity() const {
	if (SelectedVertexId == EntityId::NONE() || GetHitEntityId() == EntityId::NONE() || SelectedVertexId == GetHitEntityId())
		return false;

	const auto &FirstVertex = ES::GetEntity<VertexEntity>(SelectedVertexId);
	const auto &SecondVertex = ES::GetEntity<VertexEntity>(GetHitEntityId());

	if (FirstVertex.GraphId != SecondVertex.GraphId)
		return false;

	return !VertexCommands::Const::AreConnected(SelectedVertexId, GetHitEntityId());
}
