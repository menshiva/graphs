#include "GraphsRenderer.h"
#include "Commands/GraphCommands.h"
#include "Kismet/KismetSystemLibrary.h"

DECLARE_CYCLE_STAT(TEXT("AGraphsRenderer::ExecuteCommand"), STAT_AGraphsRenderer_ExecuteCommand, GRAPHS_PERF_GRAPHS_RENDERER);
DECLARE_CYCLE_STAT(TEXT("AGraphsRenderer::MarkDirty"), STAT_AGraphsRenderer_MarkDirty, GRAPHS_PERF_GRAPHS_RENDERER);
DECLARE_CYCLE_STAT(TEXT("AGraphsRenderer::GenerateVerticesRenderData"), STAT_AGraphsRenderer_GenerateVerticesRenderData, GRAPHS_PERF_GRAPHS_RENDERER);
DECLARE_CYCLE_STAT(TEXT("AGraphsRenderer::GenerateEdgesRenderData"), STAT_AGraphsRenderer_GenerateEdgesRenderData, GRAPHS_PERF_GRAPHS_RENDERER);

AGraphsRenderer::AGraphsRenderer(const FObjectInitializer &ObjectInitializer) {
	PrimaryActorTick.bCanEverTick = false;

	VerticesRuntimeMeshComponent = ObjectInitializer.CreateDefaultSubobject<URuntimeMeshComponent>(
		this,
		TEXT("VerticesRuntimeMeshComponent")
	);
	VerticesRuntimeMeshComponent->PrimaryComponentTick.bCanEverTick = false;
	VerticesRuntimeMeshComponent->SetMobility(EComponentMobility::Static);
	VerticesRuntimeMeshComponent->SetRuntimeMeshMobility(ERuntimeMeshMobility::Static);
	VerticesRuntimeMeshComponent->SetEnableGravity(false);
	VerticesRuntimeMeshComponent->CanCharacterStepUpOn = ECB_No;
	VerticesRuntimeMeshComponent->SetCollisionProfileName("Graph");
	VerticesRuntimeMeshComponent->SetCastShadow(false);

	EdgesRuntimeMeshComponent = ObjectInitializer.CreateDefaultSubobject<URuntimeMeshComponent>(
		this,
		TEXT("EdgesRuntimeMeshComponent")
	);
	EdgesRuntimeMeshComponent->PrimaryComponentTick.bCanEverTick = false;
	EdgesRuntimeMeshComponent->SetMobility(EComponentMobility::Static);
	EdgesRuntimeMeshComponent->SetRuntimeMeshMobility(ERuntimeMeshMobility::Static);
	EdgesRuntimeMeshComponent->SetEnableGravity(false);
	EdgesRuntimeMeshComponent->CanCharacterStepUpOn = ECB_No;
	EdgesRuntimeMeshComponent->SetCollisionProfileName("Graph");
	EdgesRuntimeMeshComponent->SetCastShadow(false);
}

void AGraphsRenderer::OnConstruction(const FTransform &Transform) {
	Super::OnConstruction(Transform);
	VerticesProvider = NewObject<UVerticesRenderer>(this);
	VerticesRuntimeMeshComponent->Initialize(VerticesProvider);

	EdgesProvider = NewObject<UEdgesRenderer>(this);
	EdgesRuntimeMeshComponent->Initialize(EdgesProvider);
}

void AGraphsRenderer::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	// cleanup entity storage
	const auto &GraphStorage = ES::GetStorage<GraphEntity>();
	for (auto GraphIter = GraphStorage.CreateConstIterator(); GraphIter; ++GraphIter)
		ExecuteCommand(GraphCommands::Remove(EntityId(GraphIter.GetIndex(), GRAPH)));
	Super::EndPlay(EndPlayReason);
}

EntityId AGraphsRenderer::GetEntityIdFromHitResult(const FHitResult &HitResult) const {
	auto HitEntityId = EntityId::NONE();

	if (HitResult.bBlockingHit && HitResult.GetActor() == this) {
		if (HitResult.Component == VerticesRuntimeMeshComponent) {
			HitEntityId = EntityId(
				VerticesRuntimeMeshComponent->GetHitSource(HitResult.FaceIndex).SectionId,
				VERTEX
			);
			if (!ES::IsValid<VertexEntity>(HitEntityId))
				HitEntityId = EntityId::NONE();
		}
		else if (HitResult.Component == EdgesRuntimeMeshComponent) {
			HitEntityId = EntityId(
				EdgesRuntimeMeshComponent->GetHitSource(HitResult.FaceIndex).SectionId,
				EDGE
			);
			if (!ES::IsValid<EdgeEntity>(HitEntityId))
				HitEntityId = EntityId::NONE();
		}
	}

	return HitEntityId;
}

bool AGraphsRenderer::ExecuteCommand(GraphsRendererCommand &&Cmd, const bool MarkDirty) {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderer_ExecuteCommand);
	const auto Res = Cmd.Implementation(*this);
	if (MarkDirty)
		this->MarkDirty();
	return Res;
}

void AGraphsRenderer::MarkDirty() {
	if (ComponentsMeshDirty == 0 && ComponentsCollisionDirty == 0)
		return;

	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderer_MarkDirty);

	const bool UpdateVerticesLODs = ComponentsMeshDirty[VERTEX];
	const bool UpdateVerticesCollision = ComponentsCollisionDirty[VERTEX];
	ComponentsMeshDirty[VERTEX] = false;
	ComponentsCollisionDirty[VERTEX] = false;

	if (UpdateVerticesLODs || UpdateVerticesCollision) {
		VerticesProvider->SetRenderData(GenerateVerticesRenderData(), UpdateVerticesLODs, UpdateVerticesCollision);
		if (UpdateVerticesLODs)
			UKismetSystemLibrary::PrintString(GetWorld(), "UVertexProvider::GetSectionMeshForLOD", true, true, FLinearColor::Red);
		if (UpdateVerticesCollision)
			UKismetSystemLibrary::PrintString(GetWorld(), "UVertexProvider::GetCollisionMesh", true, true, FLinearColor::Green);
	}

	const bool UpdateEdgesLODs = ComponentsMeshDirty[EDGE];
	const bool UpdateEdgesCollision = ComponentsCollisionDirty[EDGE];
	ComponentsMeshDirty[EDGE] = false;
	ComponentsCollisionDirty[EDGE] = false;

	if (UpdateEdgesLODs || UpdateEdgesCollision) {
		EdgesProvider->SetRenderData(GenerateEdgesRenderData(), UpdateEdgesLODs, UpdateEdgesCollision);
		if (UpdateEdgesLODs)
			UKismetSystemLibrary::PrintString(GetWorld(), "UEdgesRenderer::GetSectionMeshForLOD", true, true, FLinearColor::Red);
		if (UpdateEdgesCollision)
			UKismetSystemLibrary::PrintString(GetWorld(), "UEdgesRenderer::GetCollisionMesh", true, true, FLinearColor::Green);
	}
}

RenderData AGraphsRenderer::GenerateVerticesRenderData() const {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderer_GenerateVerticesRenderData);
	const auto &VertexStorage = ES::GetStorage<VertexEntity>();

	RenderData RenderData;
	RenderData.StorageIndices.Reserve(VertexStorage.Num());
	RenderData.Positions.Reserve(VertexStorage.Num());
	RenderData.Colors.Reserve(VertexStorage.Num());

	for (auto VertexIter = VertexStorage.CreateConstIterator(); VertexIter; ++VertexIter) {
		RenderData.StorageIndices.Push(VertexIter.GetIndex());
		RenderData.Positions.Push(VertexIter->Position);
		RenderData.Colors.Push(
			VertexIter->IsHit
				? ColorConsts::BlueColor
				: VertexIter->OverrideColor != ColorConsts::OverrideColorNone
					? VertexIter->OverrideColor
					: ES::GetEntity<GraphEntity>(VertexIter->GraphId).Colorful
						? VertexIter->Color
						: ColorConsts::VertexDefaultColor
		);
	}

	return RenderData;
}

RenderData AGraphsRenderer::GenerateEdgesRenderData() const {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderer_GenerateEdgesRenderData);
	const auto &EdgesStorage = ES::GetStorage<EdgeEntity>();

	RenderData RenderData;
	RenderData.StorageIndices.Reserve(EdgesStorage.Num());
	RenderData.Positions.Reserve(EdgesStorage.Num() * 2);
	RenderData.Colors.Reserve(EdgesStorage.Num() * 2);

	for (auto EdgeIter = EdgesStorage.CreateConstIterator(); EdgeIter; ++EdgeIter) {
		const auto &Graph = ES::GetEntity<GraphEntity>(EdgeIter->GraphId);

		const auto &FirstVertex = ES::GetEntity<VertexEntity>(EdgeIter->VerticesIds[0]);
		const auto &SecondVertex = ES::GetEntity<VertexEntity>(EdgeIter->VerticesIds[1]);

		auto FirstColor = FirstVertex.Color;
		auto SecondColor = SecondVertex.Color;

		if (EdgeIter->IsHit) {
			FirstColor = ColorConsts::BlueColor;
			SecondColor = FirstColor;
		}
		else if (EdgeIter->OverrideColor != ColorConsts::OverrideColorNone) {
			FirstColor = EdgeIter->OverrideColor;
			SecondColor = FirstColor;
		}
		else if (!Graph.Colorful) {
			FirstColor = ColorConsts::VertexDefaultColor;
			SecondColor = FirstColor;
		}

		RenderData.StorageIndices.Push(EdgeIter.GetIndex());

		RenderData.Positions.Push(FirstVertex.Position);
		RenderData.Positions.Push(SecondVertex.Position);

		RenderData.Colors.Push(FirstColor);
		RenderData.Colors.Push(SecondColor);
	}

	return RenderData;
}
