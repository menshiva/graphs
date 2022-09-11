#include "VertexCommands.h"

VertexCommands::Create::Create(
	const EntityId *GraphId, EntityId *NewVertexId,
	const uint32_t UserId,
	const FVector &Position,
	const FLinearColor &Color
) : Command([=] (EntityStorage &Storage) -> bool {
	const auto VertexId = Storage.NewEntity<VertexEntity>();

	auto &Graph = Storage.GetEntityMut<GraphEntity>(*GraphId);
	check(!Graph.VertexUserIdToEntityId.Contains(UserId));
	Graph.VertexUserIdToEntityId.Add(UserId, VertexId);
	check(!Graph.VerticesIds.Contains(VertexId));
	Graph.VerticesIds.Push(VertexId);

	auto &Vertex = Storage.GetEntityMut<VertexEntity>(VertexId);

	Vertex.GraphId = *GraphId;
	Vertex.Selection = EntitySelection::NONE;

	Vertex.UserId = UserId;
	Vertex.Position = Position;
	Vertex.Color = Color;

	if (NewVertexId)
		*NewVertexId = VertexId;

	return true;
}) {}

VertexCommands::SetSelection::SetSelection(
	const EntityId &VertexId,
	const EntitySelection NewSelection
) : Command([=] (EntityStorage &Storage) -> bool {
	auto &Vertex = Storage.GetEntityMut<VertexEntity>(VertexId);
	if (Vertex.Selection == NewSelection)
		return false;
	Vertex.Selection = NewSelection;
	return true;
}) {}
