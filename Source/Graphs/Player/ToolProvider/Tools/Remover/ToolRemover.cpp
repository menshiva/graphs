#include "ToolRemover.h"
#include "ToolRemoverPanelWidget.h"
#include "Graphs/EntityStorage/Commands/EdgeCommands.h"
#include "Graphs/EntityStorage/Commands/GraphCommands.h"
#include "Graphs/EntityStorage/Commands/VertexCommands.h"

DECLARE_CYCLE_STAT(TEXT("UToolRemover::OnRightTriggerAction"), STAT_UToolRemover_OnRightTriggerAction, STATGROUP_GRAPHS_PERF);

UToolRemover::UToolRemover() : UTool(
	"Remove",
	TEXT("/Game/Graphs/UI/Icons/Remove"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolRemoverPanel")
) {
	SetSupportedEntities({GRAPH, EDGE, VERTEX});
}

void UToolRemover::OnAttach() {
	Super::OnAttach();
	GetVrRightController()->SetLaserActive(true);
}

void UToolRemover::OnDetach() {
	Super::OnDetach();
	GetVrRightController()->SetLaserActive(false);
	DeselectEntities();
}

bool UToolRemover::OnRightTriggerAction(const bool IsPressed) {
	SCOPE_CYCLE_COUNTER(STAT_UToolRemover_OnRightTriggerAction);

	if (IsPressed && GetHitEntityId() != EntityId::NONE()) {
		if (ES::IsValid<VertexEntity>(GetHitEntityId())) {
			const auto &Vertex = ES::GetEntity<VertexEntity>(GetHitEntityId());
			auto GraphRemoveData = GraphsRemoveData.Find(Vertex.GraphId);

			if (GraphRemoveData && GraphRemoveData->VerticesToRemove.Contains(GetHitEntityId())) {
				GraphRemoveData->VerticesToRemove.Remove(GetHitEntityId());
				if (GraphRemoveData->VerticesToRemove.Num() == 0 && GraphRemoveData->EdgesToRemove.Num() == 0)
					GraphsRemoveData.Remove(Vertex.GraphId);
				VertexCommands::Mutable::SetOverrideColor(GetHitEntityId(), ColorConsts::OverrideColorNone);
			}
			else {
				if (!GraphRemoveData)
					GraphRemoveData = &GraphsRemoveData.Add(Vertex.GraphId);
				GraphRemoveData->VerticesToRemove.Add(GetHitEntityId());
				VertexCommands::Mutable::SetOverrideColor(GetHitEntityId(), ColorConsts::RedColor);
			}
		}
		else if (ES::IsValid<EdgeEntity>(GetHitEntityId())) {
			const auto &Edge = ES::GetEntity<EdgeEntity>(GetHitEntityId());
			auto GraphRemoveData = GraphsRemoveData.Find(Edge.GraphId);

			if (GraphRemoveData && GraphRemoveData->EdgesToRemove.Contains(GetHitEntityId())) {
				GraphRemoveData->EdgesToRemove.Remove(GetHitEntityId());
				if (GraphRemoveData->VerticesToRemove.Num() == 0 && GraphRemoveData->EdgesToRemove.Num() == 0)
					GraphsRemoveData.Remove(Edge.GraphId);
				EdgeCommands::Mutable::SetOverrideColor(GetHitEntityId(), ColorConsts::OverrideColorNone);
			}
			else {
				if (!GraphRemoveData)
					GraphRemoveData = &GraphsRemoveData.Add(Edge.GraphId);
				GraphRemoveData->EdgesToRemove.Add(GetHitEntityId());
				EdgeCommands::Mutable::SetOverrideColor(GetHitEntityId(), ColorConsts::RedColor);
			}
		}
		else {
			const auto &Graph = ES::GetEntity<GraphEntity>(GetHitEntityId());
			auto GraphRemoveData = GraphsRemoveData.Find(GetHitEntityId());

			if (GraphRemoveData
				&& Graph.Vertices.Num() == GraphRemoveData->VerticesToRemove.Num()
				&& Graph.Edges.Num() == GraphRemoveData->EdgesToRemove.Num())
			{
				GraphsRemoveData.Remove(GetHitEntityId());
				GraphCommands::Mutable::SetOverrideColor(GetHitEntityId(), ColorConsts::OverrideColorNone);
			}
			else {
				if (!GraphRemoveData)
					GraphRemoveData = &GraphsRemoveData.Add(GetHitEntityId());
				GraphRemoveData->VerticesToRemove.Append(Graph.Vertices);
				GraphRemoveData->EdgesToRemove.Append(Graph.Edges);
				GraphCommands::Mutable::SetOverrideColor(GetHitEntityId(), ColorConsts::RedColor);
			}
		}

		// we don't need to mark renderer dirty here because we assume that entity is hit
		// so renderer will be marked dirty anyway when we unhit entity

		GetToolPanel<UToolRemoverPanelWidget>()->SetButtonsEnabled(GraphsRemoveData.Num() > 0);
		return true;
	}

	return Super::OnRightTriggerAction(IsPressed);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UToolRemover::RemoveSelectedEntities() {
	const auto GraphsRenderer = GetGraphsRenderers();
	const auto RemoverToolPanel = GetToolPanel<UToolRemoverPanelWidget>();
	const auto RightVrController = GetVrRightController();
	RightVrController->SetLaserActive(false);
	RemoverToolPanel->SetLoadingStatus(true);

	AsyncTask(
		ENamedThreads::AnyBackgroundHiPriTask,
		[GraphsRemoveData(MoveTemp(GraphsRemoveData)), GraphsRenderer, RemoverToolPanel, RightVrController] {
			for (const auto &P : GraphsRemoveData) {
				const auto &Graph = ES::GetEntity<GraphEntity>(P.Key);
				const auto &GraphRemoveData = P.Value;

				if (GraphRemoveData.VerticesToRemove.Num() == Graph.Vertices.Num()) {
					GraphsRenderer->RemoveGraphChunks(P.Key);
					GraphCommands::Mutable::Remove(P.Key);
				}
				else {
					for (const auto VertexId : GraphRemoveData.VerticesToRemove) {
						GraphsRenderer->RemoveVertexFromChunk(VertexId);
						VertexCommands::Mutable::Remove(VertexId);
					}
					for (const auto EdgeId : GraphRemoveData.EdgesToRemove) {
						if (ES::IsValid<EdgeEntity>(EdgeId)) {
							GraphsRenderer->RemoveEdgeFromChunk(EdgeId);
							EdgeCommands::Mutable::Remove(EdgeId);
						}
					}
					GraphsRenderer->RedrawGraphChunksIfDirty(P.Key);
					GraphCommands::Mutable::CompactSets(P.Key);
				}
			}

			Utils::DoOnGameThread([RemoverToolPanel, RightVrController] {
				RemoverToolPanel->SetLoadingStatus(false);
				RightVrController->SetLaserActive(true);
			});
		}
	);

	check(GraphsRemoveData.Num() == 0);
}

void UToolRemover::DeselectEntities() {
	for (const auto &P : GraphsRemoveData) {
		const auto &Graph = ES::GetEntity<GraphEntity>(P.Key);
		const auto &GraphRemoveData = P.Value;

		if (GraphRemoveData.VerticesToRemove.Num() == Graph.Vertices.Num()
			|| GraphRemoveData.EdgesToRemove.Num() == Graph.Edges.Num())
		{
			GraphCommands::Mutable::SetOverrideColor(P.Key, ColorConsts::OverrideColorNone);
			GetGraphsRenderers()->MarkGraphDirty(
				P.Key,
				true, false,
				true, false
			);
			GetGraphsRenderers()->RedrawGraphChunksIfDirty(P.Key);
		}
		else {
			for (const auto VertexId : GraphRemoveData.VerticesToRemove) {
				VertexCommands::Mutable::SetOverrideColor(VertexId, ColorConsts::OverrideColorNone);
				GetGraphsRenderers()->MarkVertexDirty(VertexId, true, false, false);
			}
			for (const auto EdgeId : GraphRemoveData.EdgesToRemove) {
				EdgeCommands::Mutable::SetOverrideColor(EdgeId, ColorConsts::OverrideColorNone);
				GetGraphsRenderers()->MarkEdgeDirty(EdgeId, true, false, false);
			}
			GetGraphsRenderers()->RedrawGraphChunksIfDirty(P.Key);
		}
	}
	GraphsRemoveData.Reset();
}
