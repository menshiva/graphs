#include "ToolRemover.h"
#include "ToolRemoverPanelWidget.h"
#include "Graphs/GraphsRenderers/Commands/EdgeCommands.h"
#include "Graphs/GraphsRenderers/Commands/GraphCommands.h"
#include "Graphs/GraphsRenderers/Commands/VertexCommands.h"

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
				GetGraphsRenderers()->ExecuteCommand(VertexCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::OverrideColorNone
				));
			}
			else {
				if (!GraphRemoveData)
					GraphRemoveData = &GraphsRemoveData.Add(Vertex.GraphId);
				GraphRemoveData->VerticesToRemove.Add(GetHitEntityId());
				GetGraphsRenderers()->ExecuteCommand(VertexCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::RedColor
				));
			}
		}
		else if (ES::IsValid<EdgeEntity>(GetHitEntityId())) {
			const auto &Edge = ES::GetEntity<EdgeEntity>(GetHitEntityId());
			auto GraphRemoveData = GraphsRemoveData.Find(Edge.GraphId);

			if (GraphRemoveData && GraphRemoveData->EdgesToRemove.Contains(GetHitEntityId())) {
				GraphRemoveData->EdgesToRemove.Remove(GetHitEntityId());
				if (GraphRemoveData->VerticesToRemove.Num() == 0 && GraphRemoveData->EdgesToRemove.Num() == 0)
					GraphsRemoveData.Remove(Edge.GraphId);
				GetGraphsRenderers()->ExecuteCommand(EdgeCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::OverrideColorNone
				));
			}
			else {
				if (!GraphRemoveData)
					GraphRemoveData = &GraphsRemoveData.Add(Edge.GraphId);
				GraphRemoveData->EdgesToRemove.Add(GetHitEntityId());
				GetGraphsRenderers()->ExecuteCommand(EdgeCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::RedColor
				));
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
				GetGraphsRenderers()->ExecuteCommand(GraphCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::OverrideColorNone
				));
			}
			else {
				if (!GraphRemoveData)
					GraphRemoveData = &GraphsRemoveData.Add(GetHitEntityId());
				GraphRemoveData->VerticesToRemove.Append(Graph.Vertices);
				GraphRemoveData->EdgesToRemove.Append(Graph.Edges);
				GetGraphsRenderers()->ExecuteCommand(GraphCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::RedColor
				));
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
	RemoverToolPanel->SetButtonsEnabled(false);

	for (const auto &P : GraphsRemoveData) {
		const auto &Graph = ES::GetEntity<GraphEntity>(P.Key);
		const auto &GraphRemoveData = P.Value;
		if (GraphRemoveData.VerticesToRemove.Num() == Graph.Vertices.Num())
			GraphsRenderer->RemoveGraphRenderer(P.Key);
	}

	AsyncTask(
		ENamedThreads::AnyBackgroundHiPriTask,
		[GraphsRemoveData(MoveTemp(GraphsRemoveData)), GraphsRenderer, RemoverToolPanel, RightVrController] {
			for (const auto &P : GraphsRemoveData) {
				const auto &Graph = ES::GetEntity<GraphEntity>(P.Key);
				const auto &GraphRemoveData = P.Value;

				if (GraphRemoveData.VerticesToRemove.Num() == Graph.Vertices.Num()) {
					GraphsRenderer->ExecuteCommand(GraphCommands::Remove(P.Key));
				}
				else {
					for (const auto VertexId : GraphRemoveData.VerticesToRemove)
						GraphsRenderer->ExecuteCommand(VertexCommands::Remove(VertexId));
					for (const auto EdgeId : GraphRemoveData.EdgesToRemove)
						if (ES::IsValid<EdgeEntity>(EdgeId))
							GraphsRenderer->ExecuteCommand(EdgeCommands::Remove(EdgeId));
				}
			}

			Utils::DoOnGameThread([=] {
				GraphsRenderer->RedrawIfDirty();
				RemoverToolPanel->SetLoadingStatus(false);
				RightVrController->SetLaserActive(true);
			});
		}
	);

	check(GraphsRemoveData.Num() == 0);
}

void UToolRemover::DeselectEntities() {
	for (const auto &P : GraphsRemoveData)
		GetGraphsRenderers()->ExecuteCommand(GraphCommands::SetOverrideColor(P.Key, ColorConsts::OverrideColorNone));
	GetGraphsRenderers()->RedrawIfDirty();
	GraphsRemoveData.Reset();
	GetToolPanel<UToolRemoverPanelWidget>()->SetButtonsEnabled(false);
}
