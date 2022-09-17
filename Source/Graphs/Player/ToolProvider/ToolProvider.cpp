#include "ToolProvider.h"
#include "Graphs/GraphRenderer/Commands/EdgeCommands.h"
#include "Graphs/GraphRenderer/Commands/GraphCommands.h"
#include "Graphs/GraphRenderer/Commands/VertexCommands.h"
#include "Kismet/GameplayStatics.h"
#include "Tools/Importer/ToolImporter.h"
#include "Tools/Exporter/ToolExporter.h"
#include "Tools/Manipulator/ToolManipulator.h"
#include "Tools/Remover/ToolRemover.h"

DECLARE_CYCLE_STAT(TEXT("UToolProvider::Tick"), STAT_UToolProvider_Tick, STATGROUP_GRAPHS_PERF);

UToolProvider::UToolProvider(const FObjectInitializer &ObjectInitializer) : UActorComponent(ObjectInitializer) {
	PrimaryComponentTick.bCanEverTick = true;

	RegisterTool<UToolImporter>(ObjectInitializer, "Tool Importer");
	RegisterTool<UToolExporter>(ObjectInitializer, "Tool Exporter");
	RegisterTool<UToolManipulator>(ObjectInitializer, "Tool Manipulator");
	RegisterTool<UToolRemover>(ObjectInitializer, "Tool Remover");
}

void UToolProvider::SetHitResult(const FHitResult &NewHitResult) {
	HitResult = NewHitResult;
	auto NewHitEntityId = GetGraphsRenderer()->GetEntityIdFromHitResult(NewHitResult);

	if (NewHitEntityId != EntityId::NONE() && VrPawn->GetRightVrController()->GetSelectionMode() == SelectionMode::GRAPH) {
		if (ES::IsValid<VertexEntity>(NewHitEntityId))
			NewHitEntityId = ES::GetEntity<VertexEntity>(NewHitEntityId).GraphId;
		else
			NewHitEntityId = ES::GetEntity<EdgeEntity>(NewHitEntityId).GraphId;
		if (!ES::IsValid<GraphEntity>(NewHitEntityId))
			NewHitEntityId = EntityId::NONE();
	}

	if (NewHitEntityId != HitEntityId) {
		if (HitEntityId != EntityId::NONE())
			ExecuteHitCommandBasedOnHitEntity(false);

		HitEntityId = EntityId::NONE();

		if (NewHitEntityId != EntityId::NONE() && ActiveTool.IsValid() && ActiveTool->SupportsEntity(NewHitEntityId)) {
			HitEntityId = NewHitEntityId;
			ExecuteHitCommandBasedOnHitEntity(true);
		}

		GetGraphsRenderer()->MarkDirty();
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
	SCOPE_CYCLE_COUNTER(STAT_UToolProvider_Tick);
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

void UToolProvider::ExecuteHitCommandBasedOnHitEntity(const bool IsHit) const {
	if (ES::IsValid<VertexEntity>(HitEntityId))
		GetGraphsRenderer()->ExecuteCommand(VertexCommands::SetHit(HitEntityId, IsHit));
	else if (ES::IsValid<EdgeEntity>(HitEntityId))
		GetGraphsRenderer()->ExecuteCommand(EdgeCommands::SetHit(HitEntityId, IsHit));
	else
		GetGraphsRenderer()->ExecuteCommand(GraphCommands::SetHit(HitEntityId, IsHit));
}

void UToolProvider::BeginPlay() {
	Super::BeginPlay();
	GraphsRenderer = Cast<AGraphsRenderer>(UGameplayStatics::GetActorOfClass(GetWorld(), AGraphsRenderer::StaticClass()));
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

		EntityId GraphId = EntityId::NONE();
		GetGraphsRenderer()->ExecuteCommand(GraphCommands::Create(&GraphId, true));

		GetGraphsRenderer()->ExecuteCommand(VertexCommands::Reserve(GraphId, Positions.Num()));
		GetGraphsRenderer()->ExecuteCommand(EdgeCommands::Reserve(GraphId, Connections.Num()));

		for (size_t i = 0; i < Positions.Num(); ++i) {
			GetGraphsRenderer()->ExecuteCommand(VertexCommands::Create(
				GraphId, nullptr,
				i,
				Positions[i],
				FLinearColor::MakeRandomColor().ToFColor(false)
			));
		}

		for (size_t i = 0; i < Connections.Num(); ++i) {
			GetGraphsRenderer()->ExecuteCommand(EdgeCommands::Create(
				GraphId, nullptr,
				Connections[i].first, Connections[i].second
			));
		}

		check(ES::IsValid<GraphEntity>(GraphId));
		const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);
		check(Graph.VerticesIds.Num() == Positions.Num());
		for (const auto &VertexId : Graph.VerticesIds) {
			check(ES::IsValid<VertexEntity>(VertexId));
			const auto &Vertex = ES::GetEntity<VertexEntity>(VertexId);
			for (const auto &EdgeId : Vertex.EdgesIds) {
				check(ES::IsValid<EdgeEntity>(EdgeId));
			}
		}
		check(Graph.EdgesIds.Num() == Connections.Num());
		for (const auto &EdgeId : Graph.EdgesIds) {
			check(ES::IsValid<EdgeEntity>(EdgeId));
			const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);
			for (const auto &VertexId : Edge.VerticesIds) {
				check(ES::IsValid<VertexEntity>(VertexId));
			}
		}

		GetGraphsRenderer()->MarkDirty();
	}
}
