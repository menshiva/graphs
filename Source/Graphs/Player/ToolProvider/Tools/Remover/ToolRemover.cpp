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
		const auto RemovedEntNum = SelectedEntitiesToRemove.Remove(GetHitEntityId());
		check(RemovedEntNum <= 1);

		if (RemovedEntNum == 1) {
			if (GetEntityStorage().IsValid<VertexEntity>(GetHitEntityId())) {
				GetGraphRenderer()->PushCommand(VertexCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::OverrideColorNone
				));
			}
			else if (GetEntityStorage().IsValid<EdgeEntity>(GetHitEntityId())) {
				GetGraphRenderer()->PushCommand(EdgeCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::OverrideColorNone
				));
			}
			else if (GetEntityStorage().IsValid<GraphEntity>(GetHitEntityId())) {
				GetGraphRenderer()->PushCommand(GraphCommands::SetOverrideColor(
					GetHitEntityId(),
					ColorConsts::OverrideColorNone
				));
			}
		}
		else {
			if (GetEntityStorage().IsValid<VertexEntity>(GetHitEntityId())) {
				SelectVertexEntity(GetHitEntityId());
			}
			else if (GetEntityStorage().IsValid<EdgeEntity>(GetHitEntityId())) {
				SelectEdgeEntity(GetHitEntityId());
			}
			else if (GetEntityStorage().IsValid<GraphEntity>(GetHitEntityId())) {
				SelectGraphEntity(GetHitEntityId());
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
			GetGraphRenderer()->PushCommand(VertexCommands::Remove(SelectedEntId));
		}
		else if (GetEntityStorage().IsValid<EdgeEntity>(SelectedEntId)) {
			GetGraphRenderer()->PushCommand(EdgeCommands::Remove(SelectedEntId));
		}
		else if (GetEntityStorage().IsValid<GraphEntity>(SelectedEntId)) {
			GetGraphRenderer()->PushCommand(GraphCommands::Remove(SelectedEntId));
		}
	}
	GetGraphRenderer()->MarkDirty();
	SelectedEntitiesToRemove.Empty();
	GetToolPanel<UToolRemoverPanelWidget>()->SetButtonsEnabled(false);
}

void UToolRemover::DeselectEntities() {
	for (const auto &SelectedEntId : SelectedEntitiesToRemove) {
		if (GetEntityStorage().IsValid<VertexEntity>(SelectedEntId)) {
			GetGraphRenderer()->PushCommand(VertexCommands::SetOverrideColor(SelectedEntId, ColorConsts::OverrideColorNone));
		}
		else if (GetEntityStorage().IsValid<EdgeEntity>(SelectedEntId)) {
			GetGraphRenderer()->PushCommand(EdgeCommands::SetOverrideColor(SelectedEntId, ColorConsts::OverrideColorNone));
		}
		else if (GetEntityStorage().IsValid<GraphEntity>(SelectedEntId)) {
			GetGraphRenderer()->PushCommand(GraphCommands::SetOverrideColor(SelectedEntId, ColorConsts::OverrideColorNone));
		}
	}
	GetGraphRenderer()->MarkDirty();
	SelectedEntitiesToRemove.Empty();
	GetToolPanel<UToolRemoverPanelWidget>()->SetButtonsEnabled(false);
}

void UToolRemover::SelectVertexEntity(const EntityId &VertexId) {
	// remove parent graph from set
	const auto &Vertex = GetEntityStorage().GetEntity<VertexEntity>(VertexId);
	const auto RemoveGraphNum = SelectedEntitiesToRemove.Remove(Vertex.GraphId);
	check(RemoveGraphNum <= 1);

	if (RemoveGraphNum == 0) {
		// parent graph is not removed -> just add vertex to set
		SelectedEntitiesToRemove.Add(VertexId);
		GetGraphRenderer()->PushCommand(VertexCommands::SetOverrideColor(VertexId, ColorConsts::RedColor));
	}
	else {
		// parent graph is removed -> add all its children to set exept given VertexId
		const auto &ParentGraph = GetEntityStorage().GetEntity<GraphEntity>(Vertex.GraphId);
		for (const auto &ChildVertexId : ParentGraph.VerticesIds) {
			if (ChildVertexId != VertexId) {
				SelectedEntitiesToRemove.Add(ChildVertexId);
				GetGraphRenderer()->PushCommand(VertexCommands::SetOverrideColor(ChildVertexId, ColorConsts::RedColor));
			}
		}
		for (const auto &ChildEdgeId : ParentGraph.EdgesIds) {
			SelectedEntitiesToRemove.Add(ChildEdgeId);
			GetGraphRenderer()->PushCommand(EdgeCommands::SetOverrideColor(ChildEdgeId, ColorConsts::RedColor));
		}
		GetGraphRenderer()->PushCommand(VertexCommands::SetOverrideColor(VertexId, ColorConsts::OverrideColorNone));
	}
}

void UToolRemover::SelectEdgeEntity(const EntityId &EdgeId) {
	// remove parent graph from set
	const auto &Edge = GetEntityStorage().GetEntity<EdgeEntity>(EdgeId);
	const auto RemoveGraphNum = SelectedEntitiesToRemove.Remove(Edge.GraphId);
	check(RemoveGraphNum <= 1);

	if (RemoveGraphNum == 0) {
		// parent graph is not removed -> just add edge to set
		SelectedEntitiesToRemove.Add(EdgeId);
		GetGraphRenderer()->PushCommand(EdgeCommands::SetOverrideColor(EdgeId, ColorConsts::RedColor));
	}
	else {
		// parent graph is removed -> add all its children to set exept given EdgeId
		const auto &ParentGraph = GetEntityStorage().GetEntity<GraphEntity>(Edge.GraphId);
		for (const auto &ChildVertexId : ParentGraph.VerticesIds) {
			SelectedEntitiesToRemove.Add(ChildVertexId);
			GetGraphRenderer()->PushCommand(VertexCommands::SetOverrideColor(ChildVertexId, ColorConsts::RedColor));
		}
		for (const auto &ChildEdgeId : ParentGraph.EdgesIds) {
			if (ChildEdgeId != EdgeId) {
				SelectedEntitiesToRemove.Add(ChildEdgeId);
				GetGraphRenderer()->PushCommand(EdgeCommands::SetOverrideColor(ChildEdgeId, ColorConsts::RedColor));
			}
		}
		GetGraphRenderer()->PushCommand(EdgeCommands::SetOverrideColor(EdgeId, ColorConsts::OverrideColorNone));
	}
}

void UToolRemover::SelectGraphEntity(const EntityId &GraphId) {
	// remove given graph's children entities from set (if set contains some)
	const auto &Graph = GetEntityStorage().GetEntity<GraphEntity>(GraphId);
	for (const auto &VertexId : Graph.VerticesIds)
		SelectedEntitiesToRemove.Remove(VertexId);
	for (const auto &EdgeId : Graph.EdgesIds)
		SelectedEntitiesToRemove.Remove(EdgeId);

	// add graph entity to set
	SelectedEntitiesToRemove.Add(GraphId);
	GetGraphRenderer()->PushCommand(GraphCommands::SetOverrideColor(GraphId, ColorConsts::RedColor));
}
