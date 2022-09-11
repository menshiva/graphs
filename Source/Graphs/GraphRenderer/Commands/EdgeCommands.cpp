#include "EdgeCommands.h"
#include "VertexCommands.h"

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
	Edge.IsHit = false;
	Edge.OverrideColor = ColorConsts::OverrideColorNone;

	Edge.VerticesIds[0] = FromVertexId;
	Edge.VerticesIds[1] = ToVertexId;

	if (NewEdgeId)
		*NewEdgeId = EdgeId;

	return true;
}) {}

EdgeCommands::Remove::Remove(const EntityId &EdgeId) : Command([=] (EntityStorage &Storage) -> bool {
	if (!Storage.IsValid<EdgeEntity>(EdgeId)) return false;
	const auto &Edge = Storage.GetEntity<EdgeEntity>(EdgeId);

	// remove from associated parent graph
	auto &Graph = Storage.GetEntityMut<GraphEntity>(Edge.GraphId);
	auto CheckNum = Graph.EdgesIds.Remove(EdgeId);
	check(CheckNum == 1);

	// remove connected edges
	for (const auto &VertexId : Edge.VerticesIds) {
		if (VertexId != EntityId::NONE()) {
			CheckNum = Storage.GetEntityMut<VertexEntity>(VertexId).EdgesIds.Remove(EdgeId);
			check(CheckNum == 1);
		}
	}

	Storage.RemoveEntity<EdgeEntity>(EdgeId);
	return true;
}) {}

EdgeCommands::SetHit::SetHit(
	const EntityId &EdgeId,
	const bool IsHit
) : Command([=] (EntityStorage &Storage) -> bool {
	auto &Edge = Storage.GetEntityMut<EdgeEntity>(EdgeId);

	if (Edge.IsHit == IsHit)
		return false;
	Edge.IsHit = IsHit;

	return true;
}) {}

EdgeCommands::SetOverrideColor::SetOverrideColor(
	const EntityId &EdgeId,
	const FLinearColor &OverrideColor
) : Command([=] (EntityStorage &Storage) -> bool {
	auto &Edge = Storage.GetEntityMut<EdgeEntity>(EdgeId);

	if (Edge.OverrideColor == OverrideColor)
		return false;
	Edge.OverrideColor = OverrideColor;

	return true;
}) {}

EdgeCommands::Move::Move(
	const EntityId &EdgeId,
	const FVector &Delta
) : Command([=] (EntityStorage &Storage) -> bool {
	const auto &Edge = Storage.GetEntity<EdgeEntity>(EdgeId);

	for (const auto &VertexId : Edge.VerticesIds)
		ExecuteSubCommand(VertexCommands::Move(VertexId, Delta), Storage);

	return true;
}) {}
