#include "ToolProvider.h"
#include "Graphs/EntityStorage/Commands/EdgeCommands.h"
#include "Graphs/EntityStorage/Commands/GraphCommands.h"
#include "Graphs/EntityStorage/Commands/VertexCommands.h"
#include "Kismet/GameplayStatics.h"
#include "Tools/Creator/ToolCreator.h"
#include "Tools/Editor/ToolEditor.h"
#include "Tools/Importer/ToolImporter.h"
#include "Tools/Exporter/ToolExporter.h"
#include "Tools/Manipulator/ToolManipulator.h"
#include "Tools/Remover/ToolRemover.h"

DECLARE_CYCLE_STAT(TEXT("UToolProvider::Tick"), STAT_UToolProvider_Tick, STATGROUP_GRAPHS_PERF);

UToolProvider::UToolProvider(const FObjectInitializer &ObjectInitializer) : USceneComponent(ObjectInitializer) {
	PrimaryComponentTick.bCanEverTick = true;
	RegisterTool<UToolImporter>(ObjectInitializer, "Tool Importer");
	RegisterTool<UToolExporter>(ObjectInitializer, "Tool Exporter");
	RegisterTool<UToolCreator>(ObjectInitializer, "Tool Creator");
	RegisterTool<UToolEditor>(ObjectInitializer, "Tool Editor");
	RegisterTool<UToolManipulator>(ObjectInitializer, "Tool Manipulator");
	RegisterTool<UToolRemover>(ObjectInitializer, "Tool Remover");
}

void UToolProvider::SetHitResult(const FHitResult &NewHitResult) {
	HitResult = NewHitResult;
	auto NewHitEntityId = GetGraphsRenderers()->GetEntityIdFromHitResult(NewHitResult);

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

		if (NewHitEntityId != EntityId::NONE()
			&& (!ActiveTool.IsValid()
				|| (ActiveTool.IsValid() && ActiveTool->SupportsEntity(NewHitEntityId))))
		{
			HitEntityId = NewHitEntityId;
			ExecuteHitCommandBasedOnHitEntity(true);

			if (ES::IsValid<VertexEntity>(HitEntityId)) {
				const auto &Vertex = ES::GetEntity<VertexEntity>(HitEntityId);
				auto ColorStr = "#" + Vertex.Color.ToHex();
				ColorStr.RemoveAt(ColorStr.Len() - 2, 2, false); // removes alpha channel
				GetVrPawn()->GetMenuWidget()->SetHitEntity(
					"Vertex",
					FString("Label: #") + FString::FromInt(Vertex.Label)
					+ FString("\nColor: ") + ColorStr
				);
			}
			else if (ES::IsValid<EdgeEntity>(HitEntityId)) {
				const auto &Edge = ES::GetEntity<EdgeEntity>(HitEntityId);
				const auto &FirstVertex = ES::GetEntity<VertexEntity>(Edge.ConnectedVertices[0]);
				const auto &SecondVertex = ES::GetEntity<VertexEntity>(Edge.ConnectedVertices[1]);
				GetVrPawn()->GetMenuWidget()->SetHitEntity(
					"Edge",
					FString("From: Vertex #") + FString::FromInt(FirstVertex.Label)
					+ FString("\nTo: Vertex #") + FString::FromInt(SecondVertex.Label)
					+ FString("\nWeight: ") + FString::SanitizeFloat(Edge.Weight)
				);
			}
			else {
				const auto &Graph = ES::GetEntity<GraphEntity>(HitEntityId);
				GetVrPawn()->GetMenuWidget()->SetHitEntity(
					"Graph",
					FString("Colorful: ") + (Graph.Colorful ? "true" : "false")
					+ FString("\nVertices: ") + FString::FromInt(Graph.Vertices.Num())
					+ FString("\nEdges: ") + FString::FromInt(Graph.Edges.Num())
				);
			}
		}
		else {
			GetVrPawn()->GetMenuWidget()->SetHitEntity("None", "");
		}
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
	SCOPE_CYCLE_COUNTER(STAT_UToolProvider_Tick);
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
	if (ES::IsValid<VertexEntity>(HitEntityId)) {
		VertexCommands::Mutable::SetHit(HitEntityId, IsHit);
		GetGraphsRenderers()->MarkVertexDirty(HitEntityId, true, false, false);
		GetGraphsRenderers()->RedrawChunkByVertexIfDirty(HitEntityId, false);
	}
	else if (ES::IsValid<EdgeEntity>(HitEntityId)) {
		EdgeCommands::Mutable::SetHit(HitEntityId, IsHit);
		GetGraphsRenderers()->MarkEdgeDirty(HitEntityId, true, false, false);
		GetGraphsRenderers()->RedrawChunkByEdgeIfDirty(HitEntityId, false);
	}
	else {
		GraphCommands::Mutable::SetHit(HitEntityId, IsHit);
		GetGraphsRenderers()->MarkGraphDirty(
			HitEntityId,
			true, false,
			true, false
		);
		GetGraphsRenderers()->RedrawGraphChunksIfDirty(HitEntityId);
	}
}

void UToolProvider::BeginPlay() {
	Super::BeginPlay();

	GraphsRenderers = CastChecked<AGraphsRenderers>(UGameplayStatics::GetActorOfClass(
		GetWorld(),
		AGraphsRenderers::StaticClass()
	));

#if !UE_BUILD_SHIPPING
	// is used only during the development
	{
		// when running in UE editor, singleton data is not removed, so we need to clear it manually
		ES::Clear<VertexEntity>();
		ES::Clear<EdgeEntity>();
		ES::Clear<GraphEntity>();

		// create a dummy graph
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

		const auto GraphId = GraphCommands::Mutable::Create(false);
		const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

		for (size_t i = 0; i < Positions.Num(); ++i) {
			VertexCommands::Mutable::Create(
				GraphId,
				i,
				Positions[i],
				FLinearColor::MakeRandomColor().ToFColor(false)
			);
		}

		for (size_t i = 0; i < Connections.Num(); ++i) {
			EdgeCommands::Mutable::Create(
				GraphId,
				Graph.Vertices[FSetElementId::FromInteger(Connections[i].first)],
				Graph.Vertices[FSetElementId::FromInteger(Connections[i].second)]
			);
		}

		check(ES::IsValid<GraphEntity>(GraphId));
		check(Graph.Vertices.Num() == Positions.Num());
		for (const auto VertexId : Graph.Vertices) {
			check(ES::IsValid<VertexEntity>(VertexId));
			const auto &Vertex = ES::GetEntity<VertexEntity>(VertexId);
			for (const auto EdgeId : Vertex.ConnectedEdges) {
				check(ES::IsValid<EdgeEntity>(EdgeId));
			}
		}
		check(Graph.Edges.Num() == Connections.Num());
		for (const auto EdgeId : Graph.Edges) {
			check(ES::IsValid<EdgeEntity>(EdgeId));
			const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);
			for (const auto VertexId : Edge.ConnectedVertices) {
				check(ES::IsValid<VertexEntity>(VertexId));
			}
		}

		GetGraphsRenderers()->ConstructGraphChunks(GraphId);
	}
#endif
}
