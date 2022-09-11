#include "VertexCommands.h"
#include "EdgeCommands.h"

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

VertexCommands::Remove::Remove(const EntityId &VertexId) : Command([=] (EntityStorage &Storage) -> bool {
	const auto &Vertex = Storage.GetEntity<VertexEntity>(VertexId);

	// remove from associated parent graph
	auto &Graph = Storage.GetEntityMut<GraphEntity>(Vertex.GraphId);
	auto CheckNum = Graph.VerticesIds.Remove(VertexId);
	check(CheckNum == 1);
	CheckNum = Graph.VertexUserIdToEntityId.Remove(Vertex.UserId);
	check(CheckNum == 1);

	// remove connected edges
	for (const auto &EdgeId : Vertex.EdgesIds) {
		auto &Edge = Storage.GetEntityMut<EdgeEntity>(EdgeId);

		// remove this edge from VerticesIds array of connected edge because otherwise the next
		// EdgeCommands::Remove command will invalidate this vertex EdgesIds while we iterating it
		if (Edge.VerticesIds[0] == VertexId)
			Edge.VerticesIds[0] = EntityId::NONE();
		else {
			check(Edge.VerticesIds[1] == VertexId);
			Edge.VerticesIds[1] = EntityId::NONE();
		}

		ExecuteSubCommand(EdgeCommands::Remove(EdgeId), Storage);
	}

	Storage.RemoveEntity<VertexEntity>(VertexId);
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

VertexCommands::Move::Move(
	const EntityId &VertexId,
	const FVector &Delta
) : Command([=] (EntityStorage &Storage) -> bool {
	auto &Vertex = Storage.GetEntityMut<VertexEntity>(VertexId);
	Vertex.Position += Delta;
	return true;
}) {}
