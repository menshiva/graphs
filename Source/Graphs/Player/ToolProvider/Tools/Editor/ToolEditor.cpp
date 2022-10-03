#include "ToolEditor.h"
#include "ToolEditorPanelWidget.h"
#include "Graphs/EntityStorage/Commands/GraphCommands.h"

DECLARE_CYCLE_STAT(TEXT("UToolEditor::SelectEntity"), STAT_UToolEditor_SelectEntity, STATGROUP_GRAPHS_PERF);
DECLARE_CYCLE_STAT(TEXT("UToolEditor::RestoreCache"), STAT_UToolEditor_RestoreCache, STATGROUP_GRAPHS_PERF);

UToolEditor::UToolEditor() : UTool(
	"Edit",
	TEXT("/Game/Graphs/UI/Icons/Edit"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolEditorPanel")
) {
	SetSupportedEntities({VERTEX, GRAPH});
}

void UToolEditor::OnAttach() {
	Super::OnAttach();
	GetVrRightController()->SetLaserActive(true);
}

void UToolEditor::OnDetach() {
	Super::OnDetach();
	SelectEntity(EntityId::NONE());
	GetVrRightController()->SetLaserActive(false);
}

bool UToolEditor::OnRightTriggerAction(const bool IsPressed) {
	if (IsPressed && GetHitEntityId() != EntityId::NONE()) {
		SelectEntity(GetHitEntityId());
		GetVrRightController()->SetLaserActive(false);
		return true;
	}
	return Super::OnRightTriggerAction(IsPressed);
}

void UToolEditor::SelectEntity(const EntityId NewEntityId) {
	SCOPE_CYCLE_COUNTER(STAT_UToolEditor_SelectEntity);
	SelectedEntityId = NewEntityId;

	if (ES::IsValid<GraphEntity>(SelectedEntityId)) {
		const auto &Graph = ES::GetEntity<GraphEntity>(SelectedEntityId);
		GraphEntityCache.Colorful = Graph.Colorful;
		GraphEntityCache.VerticesColors.SetNumUninitialized(Graph.Vertices.Num());
		for (size_t i = 0; i < Graph.Vertices.Num(); ++i) {
			const auto &Vertex = ES::GetEntity<VertexEntity>(Graph.Vertices[FSetElementId::FromInteger(i)]);
			GraphEntityCache.VerticesColors[i] = Vertex.Color;
		}
	}
	else if (ES::IsValid<VertexEntity>(SelectedEntityId)) {
		const auto &Vertex = ES::GetEntity<VertexEntity>(SelectedEntityId);
		VertexEntityCache.Color = Vertex.Color;
		VertexEntityCache.Value = Vertex.Value;
	}
	else {
		GetVrRightController()->SetLaserActive(true);
	}

	GetToolPanel<UToolEditorPanelWidget>()->Update();
}

bool UToolEditor::InvertGraphColorful() const {
	auto &Graph = ES::GetEntityMut<GraphEntity>(SelectedEntityId);
	Graph.Colorful = !Graph.Colorful;
	RedrawSelectedGraph();
	return Graph.Colorful;
}

void UToolEditor::SetGraphColor(const FColor &Color) const {
	GraphCommands::Mutable::SetColor(SelectedEntityId, Color);
	RedrawSelectedGraph();
}

void UToolEditor::RandomizeVerticesColors() const {
	GraphCommands::Mutable::RandomizeVerticesColors(SelectedEntityId);
	RedrawSelectedGraph();
}

void UToolEditor::SetVertexColor(const FColor &Color) const {
	VertexCommands::Mutable::SetColor(SelectedEntityId, Color);
	RedrawSelectedVertex();
}

void UToolEditor::RestoreCache() const {
	SCOPE_CYCLE_COUNTER(STAT_UToolEditor_RestoreCache);
	if (ES::IsValid<GraphEntity>(SelectedEntityId)) {
		GraphCommands::Mutable::SetColorful(SelectedEntityId, GraphEntityCache.Colorful);
		GraphCommands::Mutable::SetColor(SelectedEntityId, GraphEntityCache.VerticesColors);
		RedrawSelectedGraph();
	}
	else {
		check(ES::IsValid<VertexEntity>(SelectedEntityId));
		VertexCommands::Mutable::SetColor(SelectedEntityId, VertexEntityCache.Color);
		VertexCommands::Mutable::SetValue(SelectedEntityId, VertexEntityCache.Value);
		RedrawSelectedVertex();
	}
}

void UToolEditor::RedrawSelectedGraph() const {
	GetGraphsRenderers()->MarkGraphDirty(
		SelectedEntityId,
		true, false,
		true, false
	);
	GetGraphsRenderers()->RedrawGraphChunksIfDirty(SelectedEntityId);
}

void UToolEditor::RedrawSelectedVertex() const {
	GetGraphsRenderers()->MarkVertexDirty(SelectedEntityId, true, false, true);
	GetGraphsRenderers()->RedrawChunkByVertexIfDirty(SelectedEntityId, true);
}
