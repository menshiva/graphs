#include "VertexCommands.h"
#include "EdgeCommands.h"
#include "GraphCommands.h"

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
	Vertex.IsHit = false;
	Vertex.OverrideColor = ColorConsts::OverrideColorNone;

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

	if (Graph.VerticesIds.Num() > 0) {
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
	}
	else {
		ExecuteSubCommand(GraphCommands::Remove(Vertex.GraphId), Storage);
	}

	Storage.RemoveEntity<VertexEntity>(VertexId);
	return true;
}) {}

VertexCommands::SetHit::SetHit(
	const EntityId &VertexId,
	const bool IsHit
) : Command([=] (EntityStorage &Storage) -> bool {
	auto &Vertex = Storage.GetEntityMut<VertexEntity>(VertexId);

	if (Vertex.IsHit == IsHit)
		return false;
	Vertex.IsHit = IsHit;

	return true;
}) {}

VertexCommands::SetOverrideColor::SetOverrideColor(
	const EntityId &VertexId,
	const FLinearColor &OverrideColor
) : Command([=] (EntityStorage &Storage) -> bool {
	auto &Vertex = Storage.GetEntityMut<VertexEntity>(VertexId);

	if (Vertex.OverrideColor == OverrideColor)
		return false;
	Vertex.OverrideColor = OverrideColor;

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

void VertexCommands::ConstFuncs::Serialize(
	const EntityStorage &Storage,
	const EntityId &VertexId,
	rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
) {
	const auto &Vertex = Storage.GetEntity<VertexEntity>(VertexId);
	Writer.StartObject();

	Writer.Key("id");
	Writer.Uint(Vertex.UserId);

	const auto PositionStr = Vertex.Position.ToString();
	const FTCHARToUTF8 Convert(*PositionStr);
	Writer.Key("position");
	Writer.String(Convert.Get(), Convert.Length());

	// TODO: serialize color to hex

	Writer.EndObject();
}
