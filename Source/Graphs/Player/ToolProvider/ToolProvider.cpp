#include "ToolProvider.h"
#include "Graphs/GraphRenderer/Commands/EdgeCommands.h"
#include "Graphs/GraphRenderer/Commands/GraphCommands.h"
#include "Graphs/GraphRenderer/Commands/VertexCommands.h"
#include "Kismet/GameplayStatics.h"
#include "Tools/Importer/ToolImporter.h"
#include "Tools/Exporter/ToolExporter.h"
#include "Tools/Manipulator/ToolManipulator.h"
#include "Tools/Remover/ToolRemover.h"

UToolProvider::UToolProvider(const FObjectInitializer &ObjectInitializer) : UActorComponent(ObjectInitializer) {
	PrimaryComponentTick.bCanEverTick = true;

	RegisterTool<UToolImporter>(ObjectInitializer, "Tool Importer");
	RegisterTool<UToolExporter>(ObjectInitializer, "Tool Exporter");
	RegisterTool<UToolManipulator>(ObjectInitializer, "Tool Manipulator");
	RegisterTool<UToolRemover>(ObjectInitializer, "Tool Remover");
}

void UToolProvider::SetHitResult(const FHitResult &NewHitResult) {
	HitResult = NewHitResult;
	auto NewHitEntityId = EntityId::NONE();

	if (NewHitResult.bBlockingHit && NewHitResult.GetActor() == GetGraphRenderer()) {
		NewHitEntityId = GetGraphRenderer()->GetEntityIdFromCollisionData(NewHitResult.FaceIndex);
		if (GetEntityStorage().IsValid(NewHitEntityId)) {
			check(NewHitEntityId.GetSignature() == EntitySignature::VERTEX
					|| NewHitEntityId.GetSignature() == EntitySignature::EDGE);

			const auto RightController = VrPawn->GetRightVrController();
			if (RightController->GetSelectionMode() == SelectionMode::GRAPH) {
				if (NewHitEntityId.GetSignature() == EntitySignature::VERTEX)
					NewHitEntityId = GetEntityStorage().GetEntity<VertexEntity>(NewHitEntityId).GraphId;
				else
					NewHitEntityId = GetEntityStorage().GetEntity<EdgeEntity>(NewHitEntityId).GraphId;
			}

			check(GetEntityStorage().IsValid(NewHitEntityId));
		}
		else NewHitEntityId = EntityId::NONE();
	}

	if (NewHitEntityId != HitEntityId) {
		bool Set = false;
		if (GetEntityStorage().IsValid(HitEntityId)) {
			SetEntitySelection(EntitySelection::NONE);
			Set = true;
		}
		HitEntityId = EntityId::NONE();
		if (GetEntityStorage().IsValid(NewHitEntityId) && ActiveTool.IsValid() && ActiveTool->SupportsEntity(NewHitEntityId)) {
			HitEntityId = NewHitEntityId;
			SetEntitySelection(EntitySelection::HIT);
			Set = true;
		}
		if (Set)
			GetGraphRenderer()->MarkDirty();
	}
}

bool UToolProvider::OnRightTriggerAction(const bool IsPressed) {
	if (ActiveTool.IsValid())
		return ActiveTool->OnRightTriggerAction(IsPressed);
	return RightControllerInputInterface::OnRightTriggerAction(IsPressed);
}

bool UToolProvider::OnRightThumbstickY(const float Value) {
	if (ActiveTool.IsValid())
		return ActiveTool->OnRightThumbstickY(Value);
	return RightControllerInputInterface::OnRightThumbstickY(Value);
}

bool UToolProvider::OnRightThumbstickX(const float Value) {
	if (ActiveTool.IsValid())
		return ActiveTool->OnRightThumbstickX(Value);
	return RightControllerInputInterface::OnRightThumbstickX(Value);
}

bool UToolProvider::OnRightThumbstickXAction(const float Value) {
	if (ActiveTool.IsValid())
		return ActiveTool->OnRightThumbstickXAction(Value);
	return RightControllerInputInterface::OnRightThumbstickXAction(Value);
}

void UToolProvider::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction *ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (ActiveTool.IsValid())
		ActiveTool->TickTool();
}

void UToolProvider::SetActiveTool(UTool *NewTool) {
	if (ActiveTool.IsValid())
		ActiveTool->OnDetach();
	ActiveTool = NewTool;
	if (ActiveTool.IsValid())
		ActiveTool->OnAttach();
}

void UToolProvider::BeginPlay() {
	Super::BeginPlay();
	GraphRenderer = Cast<AGraphRenderer>(UGameplayStatics::GetActorOfClass(GetWorld(), AGraphRenderer::StaticClass()));
	// TODO: only for test
	{
		/*const auto ImporterTool = Cast<UToolImporter>(Tools[0]);
		FString ErrorMessage;
		const bool Result = ImporterTool->ImportGraphFromFile(
			FPaths::LaunchDir() + FileConsts::ExportDirName + "Test_8_Vertices_8_Edges.json",
			ErrorMessage
		);
		check(Result && ErrorMessage.Len() == 0);*/

		const TArray<FVector> Positions = {
			{437.109619f, 225.096985f, 50.0f},
			{748.974915f, 345.263428f, 260.0f},
			{504.859009f, -437.556763f, 460.0f},
			{969.929321f, -452.031494f, 260.0f},
			{1587.086426f, 611.200684f, 440.0f},
			{1903.230957f, 502.790161f, 650.0f},
			{1213.039551f, 60.030151f, 850.0f},
			{1560.0f, -250.0f, 650.0f},
		};
		const TArray<std::pair<uint32_t, uint32_t>> Connections = {
			{0, 1},
			{0, 2},
			{0, 3},
			{1, 2},
			{1, 4},
			{4, 5},
			{5, 6},
			{6, 4},
		};

		EntityId GraphId;
		GraphRenderer->PushCommand(GraphCommands::Create(&GraphId));
		check(GraphId == EntityId::NONE());

		for (size_t i = 0; i < Positions.Num(); ++i) {
			EntityId NewVertexId;
			GraphRenderer->PushCommand(VertexCommands::Create(
				&GraphId, &NewVertexId,
				i,
				Positions[i],
				FLinearColor::MakeRandomColor()
			));
			check(NewVertexId == EntityId::NONE());
		}

		for (size_t i = 0; i < Connections.Num(); ++i) {
			EntityId NewEdgeId;
			GraphRenderer->PushCommand(EdgeCommands::Create(
				&GraphId, &NewEdgeId,
				Connections[i].first, Connections[i].second
			));
			check(NewEdgeId == EntityId::NONE());
		}

		GraphRenderer->MarkDirty();

		check(GetEntityStorage().IsValid<GraphEntity>(GraphId));
		const auto Graph = GetEntityStorage().GetEntity<GraphEntity>(GraphId);
		check(Graph.VerticesIds.Num() == Positions.Num());
		for (const auto &VertexId : Graph.VerticesIds) {
			check(GetEntityStorage().IsValid<VertexEntity>(VertexId));
			const auto &Vertex = GetEntityStorage().GetEntity<VertexEntity>(VertexId);
			for (const auto &EdgeId : Vertex.EdgesIds) {
				check(GetEntityStorage().IsValid<EdgeEntity>(EdgeId));
			}
		}
		check(Graph.EdgesIds.Num() == Connections.Num());
		for (const auto &EdgeId : Graph.EdgesIds) {
			check(GetEntityStorage().IsValid<EdgeEntity>(EdgeId));
			const auto &Edge = GetEntityStorage().GetEntity<EdgeEntity>(EdgeId);
			for (const auto &VertexId : Edge.VerticesIds) {
				check(GetEntityStorage().IsValid<VertexEntity>(VertexId));
			}
		}
	}
}

void UToolProvider::SetEntitySelection(const EntitySelection NewSelection) const {
	switch (HitEntityId.GetSignature()) {
		case EntitySignature::VERTEX: {
			GetGraphRenderer()->PushCommand(VertexCommands::SetSelection(HitEntityId, NewSelection));
			break;
		}
		case EntitySignature::EDGE: {
			GetGraphRenderer()->PushCommand(EdgeCommands::SetSelection(HitEntityId, NewSelection));
			break;
		}
		case EntitySignature::GRAPH: {
			GetGraphRenderer()->PushCommand(GraphCommands::SetSelection(HitEntityId, NewSelection));
			break;
		}
		default: {
			check(false);
		}
	}
}
