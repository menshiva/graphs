#include "ToolRemover.h"
#include "ToolRemoverPanelWidget.h"
#include "Graphs/GraphRenderer/Commands/EdgeCommands.h"
#include "Graphs/GraphRenderer/Commands/GraphCommands.h"
#include "Graphs/GraphRenderer/Commands/VertexCommands.h"

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
				GetGraphsRenderer()->ExecuteCommand(VertexCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::OverrideColorNone
				));
			}
			else {
				if (!GraphRemoveData)
					GraphRemoveData = &GraphsRemoveData.Add(Vertex.GraphId);
				GraphRemoveData->VerticesToRemove.Add(GetHitEntityId());
				GetGraphsRenderer()->ExecuteCommand(VertexCommands::SetOverrideColor(
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
				GetGraphsRenderer()->ExecuteCommand(EdgeCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::OverrideColorNone
				));
			}
			else {
				if (!GraphRemoveData)
					GraphRemoveData = &GraphsRemoveData.Add(Edge.GraphId);
				GraphRemoveData->EdgesToRemove.Add(GetHitEntityId());
				GetGraphsRenderer()->ExecuteCommand(EdgeCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::RedColor
				));
			}
		}
		else {
			const auto &Graph = ES::GetEntity<GraphEntity>(GetHitEntityId());
			auto GraphRemoveData = GraphsRemoveData.Find(GetHitEntityId());

			if (GraphRemoveData
				&& Graph.VerticesIds.Num() == GraphRemoveData->VerticesToRemove.Num()
				&& Graph.EdgesIds.Num() == GraphRemoveData->EdgesToRemove.Num())
			{
				GraphsRemoveData.Remove(GetHitEntityId());
				GetGraphsRenderer()->ExecuteCommand(GraphCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::OverrideColorNone
				));
			}
			else {
				if (!GraphRemoveData)
					GraphRemoveData = &GraphsRemoveData.Add(GetHitEntityId());
				GraphRemoveData->VerticesToRemove.Append(Graph.VerticesIds);
				GraphRemoveData->EdgesToRemove.Append(Graph.EdgesIds);
				GetGraphsRenderer()->ExecuteCommand(GraphCommands::SetOverrideColor(
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
	const auto GraphsRenderer = GetGraphsRenderer();
	const auto RemoverToolPanel = GetToolPanel<UToolRemoverPanelWidget>();
	const auto RightVrController = GetVrRightController();
	RightVrController->SetLaserActive(false);
	RemoverToolPanel->SetLoadingStatus(true);
	RemoverToolPanel->SetButtonsEnabled(false);

	AsyncTask(
		ENamedThreads::AnyBackgroundHiPriTask,
		[GraphsRemoveData(MoveTemp(GraphsRemoveData)), GraphsRenderer, RemoverToolPanel, RightVrController] {
			for (const auto &P : GraphsRemoveData) {
				const auto &Graph = ES::GetEntity<GraphEntity>(P.Key);
				const auto &GraphRemoveData = P.Value;

				if (GraphRemoveData.VerticesToRemove.Num() == Graph.VerticesIds.Num()) {
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

			GraphsRenderer->MarkDirty();
			RemoverToolPanel->SetLoadingStatus(false);
			RightVrController->SetLaserActive(true);
		}
	);
	check(GraphsRemoveData.Num() == 0);
}

void UToolRemover::DeselectEntities() {
	for (const auto &P : GraphsRemoveData)
		GetGraphsRenderer()->ExecuteCommand(GraphCommands::SetOverrideColor(P.Key, ColorConsts::OverrideColorNone));
	GetGraphsRenderer()->MarkDirty();
	GraphsRemoveData.Reset();
	GetToolPanel<UToolRemoverPanelWidget>()->SetButtonsEnabled(false);
}
