#include "ToolRemover.h"
#include "ToolRemoverPanelWidget.h"
#include "Graphs/GraphRenderer/Commands/EdgeCommands.h"
#include "Graphs/GraphRenderer/Commands/GraphCommands.h"
#include "Graphs/GraphRenderer/Commands/VertexCommands.h"

UToolRemover::UToolRemover() : UTool(
	"Remove",
	TEXT("/Game/Graphs/UI/Icons/Remove"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolRemoverPanel")
) {
	SetSupportedEntities({EntitySignature::GRAPH, EntitySignature::EDGE, EntitySignature::VERTEX});
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
	if (IsPressed && GetHitEntityId() != EntityId::NONE()) {
		if (!GetEntityStorage().IsValid<GraphEntity>(GetHitEntityId())) {
			const auto RemovedEntNum = SelectedEntitiesToRemove.Remove(GetHitEntityId());
			check(RemovedEntNum <= 1);

			if (RemovedEntNum == 0)
				SelectedEntitiesToRemove.Add(GetHitEntityId());
			const auto &NewOverrideColor = RemovedEntNum == 0
				? ColorConsts::RedColor
				: ColorConsts::OverrideColorNone;

			if (GetEntityStorage().IsValid<VertexEntity>(GetHitEntityId())) {
				GetGraphRenderer()->ExecuteCommand(VertexCommands::SetOverrideColor(
					GetHitEntityId(),
					NewOverrideColor
				));
			}
			else {
				GetGraphRenderer()->ExecuteCommand(EdgeCommands::SetOverrideColor(
					GetHitEntityId(),
					NewOverrideColor
				));
			}
		}
		else {
			const auto &Graph = GetEntityStorage().GetEntity<GraphEntity>(GetHitEntityId());
			if (!GraphCommands::ConstFuncs::IsSetContainsGraphChildrenEntities(
				GetEntityStorage(),
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
				GetGraphRenderer()->ExecuteCommand(GraphCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::RedColor
				));
			}
			else {
				for (const auto &VertexId : Graph.VerticesIds)
					SelectedEntitiesToRemove.Remove(VertexId);
				for (const auto &EdgeId : Graph.EdgesIds)
					SelectedEntitiesToRemove.Remove(EdgeId);
				GetGraphRenderer()->ExecuteCommand(GraphCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::OverrideColorNone
				));
			}
		}

		// we don't need to mark renderer dirty here because we assume here that entity is hit
		// so renderer will be marked dirty anyway when we unhit entity
		// GetGraphRenderer()->MarkDirty();
		GetToolPanel<UToolRemoverPanelWidget>()->SetButtonsEnabled(SelectedEntitiesToRemove.Num() > 0);
		return true;
	}
	return Super::OnRightTriggerAction(IsPressed);
}

void UToolRemover::RemoveSelectedEntities() {
	for (const auto &SelectedEntId : SelectedEntitiesToRemove) {
		if (GetEntityStorage().IsValid<VertexEntity>(SelectedEntId)) {
			GetGraphRenderer()->ExecuteCommand(VertexCommands::Remove(SelectedEntId));
		}
		else if (GetEntityStorage().IsValid<EdgeEntity>(SelectedEntId)) {
			GetGraphRenderer()->ExecuteCommand(EdgeCommands::Remove(SelectedEntId));
		}
		else if (GetEntityStorage().IsValid<GraphEntity>(SelectedEntId)) {
			check(false);
		}
	}
	GetGraphRenderer()->MarkDirty();
	SelectedEntitiesToRemove.Empty();
	GetToolPanel<UToolRemoverPanelWidget>()->SetButtonsEnabled(false);
}

void UToolRemover::DeselectEntities() {
	for (const auto &SelectedEntId : SelectedEntitiesToRemove) {
		if (GetEntityStorage().IsValid<VertexEntity>(SelectedEntId)) {
			GetGraphRenderer()->ExecuteCommand(VertexCommands::SetOverrideColor(SelectedEntId, ColorConsts::OverrideColorNone));
		}
		else if (GetEntityStorage().IsValid<EdgeEntity>(SelectedEntId)) {
			GetGraphRenderer()->ExecuteCommand(EdgeCommands::SetOverrideColor(SelectedEntId, ColorConsts::OverrideColorNone));
		}
		else if (GetEntityStorage().IsValid<GraphEntity>(SelectedEntId)) {
			check(false);
		}
	}
	GetGraphRenderer()->MarkDirty();
	SelectedEntitiesToRemove.Empty();
	GetToolPanel<UToolRemoverPanelWidget>()->SetButtonsEnabled(false);
}
