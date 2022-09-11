#include "EdgeCommands.h"

EdgeCommands::Create::Create(
	const EntityId *GraphId, EntityId *NewEdgeId,
	const uint32_t FromVertexUserId, const uint32_t ToVertexUserId
) : Command([=] (EntityStorage &Storage) -> bool {
	const auto EdgeId = Storage.NewEntity<EdgeEntity>();

	auto &Graph = Storage.GetEntityMut<GraphEntity>(*GraphId);
	check(!Graph.EdgesIds.Contains(EdgeId));
	Graph.EdgesIds.Push(EdgeId);

	const auto &FromVertexId = Graph.VertexUserIdToEntityId.FindChecked(FromVertexUserId);
	auto &FromVertex = Storage.GetEntityMut<VertexEntity>(FromVertexId);
	check(!FromVertex.EdgesIds.Contains(EdgeId));
	FromVertex.EdgesIds.Push(EdgeId);
	
	const auto &ToVertexId = Graph.VertexUserIdToEntityId.FindChecked(ToVertexUserId);
	auto &ToVertex = Storage.GetEntityMut<VertexEntity>(ToVertexId);
	check(!ToVertex.EdgesIds.Contains(EdgeId));
	ToVertex.EdgesIds.Push(EdgeId);

	auto &Edge = Storage.GetEntityMut<EdgeEntity>(EdgeId);

	Edge.GraphId = *GraphId;
	Edge.Selection = EntitySelection::NONE;

	Edge.VerticesIds[0] = FromVertexId;
	Edge.VerticesIds[1] = ToVertexId;

	if (NewEdgeId)
		*NewEdgeId = EdgeId;

	return true;
}) {}

EdgeCommands::SetSelection::SetSelection(
	const EntityId &EdgeId,
	const EntitySelection NewSelection
) : Command([=] (EntityStorage &Storage) -> bool {
	auto &Edge = Storage.GetEntityMut<EdgeEntity>(EdgeId);
	if (Edge.Selection == NewSelection)
		return false;
	Edge.Selection = NewSelection;
	return true;
}) {}
