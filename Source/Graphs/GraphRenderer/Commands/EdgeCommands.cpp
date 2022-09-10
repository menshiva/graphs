#include "EdgeCommands.h"

EdgeCommands::Create::Create(
	const EntityId *GraphId, EntityId *NewEdgeId,
	const uint32_t FromVertexUserId, const uint32_t ToVertexUserId,
	const FLinearColor& Color
) : Command([=] (EntityStorage &Storage) {
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
	Edge.Selection = EdgeEntity::SelectionType::NONE;

	Edge.VerticesIds[0] = FromVertexId;
	Edge.VerticesIds[1] = ToVertexId;
	Edge.Color = Color;

	if (NewEdgeId)
		*NewEdgeId = EdgeId;
}) {}
