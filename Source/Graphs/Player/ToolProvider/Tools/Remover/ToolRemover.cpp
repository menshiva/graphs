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

// TODO: test on multiple graphs

bool UToolRemover::OnRightTriggerAction(const bool IsPressed) {
	SCOPE_CYCLE_COUNTER(STAT_UToolRemover_OnRightTriggerAction);

	if (IsPressed && GetHitEntityId() != EntityId::NONE()) {
		if (!ES::IsValid<GraphEntity>(GetHitEntityId())) {
			const auto RemovedEntNum = SelectedEntitiesToRemove.Remove(GetHitEntityId());
			check(RemovedEntNum <= 1);

			if (RemovedEntNum == 0)
				SelectedEntitiesToRemove.Add(GetHitEntityId());
			const auto &NewOverrideColor = RemovedEntNum == 0
				? ColorConsts::RedColor
				: ColorConsts::OverrideColorNone;

			if (ES::IsValid<VertexEntity>(GetHitEntityId())) {
				GetGraphsRenderer()->ExecuteCommand(VertexCommands::SetOverrideColor(
					GetHitEntityId(),
					NewOverrideColor
				));
			}
			else {
				GetGraphsRenderer()->ExecuteCommand(EdgeCommands::SetOverrideColor(
					GetHitEntityId(),
					NewOverrideColor
				));
			}
		}
		else {
			const auto &Graph = ES::GetEntity<GraphEntity>(GetHitEntityId());
			if (!GraphCommands::ConstFuncs::IsSetContainsGraphChildrenEntities(
				GetHitEntityId(),
				SelectedEntitiesToRemove
			)) {
				SelectedEntitiesToRemove.Reserve(
					SelectedEntitiesToRemove.Num() + Graph.VerticesIds.Num() + Graph.EdgesIds.Num()
				);
				for (const auto &VertexId : Graph.VerticesIds)
					SelectedEntitiesToRemove.Add(VertexId);
				for (const auto &EdgeId : Graph.EdgesIds)
					SelectedEntitiesToRemove.Add(EdgeId);
				GetGraphsRenderer()->ExecuteCommand(GraphCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::RedColor
				));
			}
			else {
				for (const auto &VertexId : Graph.VerticesIds)
					SelectedEntitiesToRemove.Remove(VertexId);
				for (const auto &EdgeId : Graph.EdgesIds)
					SelectedEntitiesToRemove.Remove(EdgeId);
				GetGraphsRenderer()->ExecuteCommand(GraphCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::OverrideColorNone
				));
			}
		}

		// we don't need to mark renderer dirty here because we assume here that entity is hit
		// so renderer will be marked dirty anyway when we unhit entity
		// MarkGraphRendererDirty();

		GetToolPanel<UToolRemoverPanelWidget>()->SetButtonsEnabled(SelectedEntitiesToRemove.Num() > 0);
		return true;
	}

	return Super::OnRightTriggerAction(IsPressed);
}

void UToolRemover::RemoveSelectedEntities() {
	for (const auto &SelectedEntId : SelectedEntitiesToRemove) {
		if (ES::IsValid<VertexEntity>(SelectedEntId)) {
			GetGraphsRenderer()->ExecuteCommand(VertexCommands::Remove(SelectedEntId));
		}
		else if (ES::IsValid<EdgeEntity>(SelectedEntId)) {
			GetGraphsRenderer()->ExecuteCommand(EdgeCommands::Remove(SelectedEntId));
		}
		else if (ES::IsValid<GraphEntity>(SelectedEntId)) {
			check(false);
		}
	}
	GetGraphsRenderer()->MarkDirty();
	SelectedEntitiesToRemove.Reset();
	GetToolPanel<UToolRemoverPanelWidget>()->SetButtonsEnabled(false);
}

void UToolRemover::DeselectEntities() {
	for (const auto &SelectedEntId : SelectedEntitiesToRemove) {
		if (ES::IsValid<VertexEntity>(SelectedEntId)) {
			GetGraphsRenderer()->ExecuteCommand(VertexCommands::SetOverrideColor(
				SelectedEntId,
				ColorConsts::OverrideColorNone
			));
		}
		else if (ES::IsValid<EdgeEntity>(SelectedEntId)) {
			GetGraphsRenderer()->ExecuteCommand(EdgeCommands::SetOverrideColor(
				SelectedEntId,
				ColorConsts::OverrideColorNone
			));
		}
		else if (ES::IsValid<GraphEntity>(SelectedEntId)) {
			check(false);
		}
	}
	GetGraphsRenderer()->MarkDirty();
	SelectedEntitiesToRemove.Reset();
	GetToolPanel<UToolRemoverPanelWidget>()->SetButtonsEnabled(false);
}
