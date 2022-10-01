#include "EdgeCommands.h"
#include "VertexCommands.h"

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Mutable::Create"), STAT_EdgeCommands_Mutable_Create, STATGROUP_GRAPHS_PERF_COMMANDS);
EntityId EdgeCommands::Mutable::Create(const EntityId GraphId, const EntityId FromVertexId, const EntityId ToVertexId) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Mutable_Create);

	// create new edge entity
	const auto EdgeId = ES::NewEntity<EdgeEntity>();

	// fill new entity with given properties
	auto &Edge = ES::GetEntityMut<EdgeEntity>(EdgeId);
	Edge.GraphId = GraphId;
	Edge.IsHit = false;
	Edge.OverrideColor = ColorConsts::OverrideColorNone;
	Edge.ConnectedVertices[0] = FromVertexId;
	Edge.ConnectedVertices[1] = ToVertexId;

	// add new entity to parent graph
	auto &Graph = ES::GetEntityMut<GraphEntity>(GraphId);
	bool AlreadyInSet = false;
	Graph.EdgesHashes.Add(Const::ComputeHash(Edge, false), &AlreadyInSet);
	check(!AlreadyInSet);
	check(!Graph.EdgesHashes.Contains(Const::ComputeHash(Edge, true)));
	Graph.Edges.Add(EdgeId, &AlreadyInSet);
	check(!AlreadyInSet);

	// add to connected vertices
	auto &FirstVertex = ES::GetEntityMut<VertexEntity>(FromVertexId);
	auto &SecondVertex = ES::GetEntityMut<VertexEntity>(ToVertexId);
	FirstVertex.ConnectedEdges.Add(EdgeId, &AlreadyInSet);
	check(!AlreadyInSet);
	SecondVertex.ConnectedEdges.Add(EdgeId, &AlreadyInSet);
	check(!AlreadyInSet);
	check(FirstVertex.GraphId == SecondVertex.GraphId);

	return EdgeId;
}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Mutable::Remove"), STAT_EdgeCommands_Mutable_Remove, STATGROUP_GRAPHS_PERF_COMMANDS);
void EdgeCommands::Mutable::Remove(const EntityId EdgeId) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Mutable_Remove);
	const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);

	// remove from associated parent graph
	auto &Graph = ES::GetEntityMut<GraphEntity>(Edge.GraphId);
	auto CheckNum = Graph.EdgesHashes.Remove(Const::ComputeHash(Edge, false));
	check(CheckNum == 1);
	check(!Graph.EdgesHashes.Contains(Const::ComputeHash(Edge, true)));
	CheckNum = Graph.Edges.Remove(EdgeId);
	check(CheckNum == 1);

	// remove from connected vertices
	for (const auto VertexId : Edge.ConnectedVertices)
		ES::GetEntityMut<VertexEntity>(VertexId).ConnectedEdges.Remove(EdgeId);

	ES::RemoveEntity<EdgeEntity>(EdgeId);
}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Mutable::Move"), STAT_EdgeCommands_Mutable_Move, STATGROUP_GRAPHS_PERF_COMMANDS);
void EdgeCommands::Mutable::Move(const EntityId EdgeId, const FVector &Delta) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Mutable_Move);
	const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);
	VertexCommands::Mutable::Move(Edge.ConnectedVertices[0], Delta);
	VertexCommands::Mutable::Move(Edge.ConnectedVertices[1], Delta);
}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Mutable::Deserialize"), STAT_EdgeCommands_Mutable_Deserialize, STATGROUP_GRAPHS_PERF_COMMANDS);
bool EdgeCommands::Mutable::Deserialize(const rapidjson::Value &DomEdge, const EntityId GraphId, FString &ErrorMessage) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Mutable_Deserialize);

	if (!DomEdge.IsObject()) {
		ErrorMessage = "Should be an object.";
		return false;
	}
	
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	EdgeEntity NewEdge;

	// from vertex id
	{
		const auto &FromIdMember = DomEdge.FindMember("from_id");
		if (FromIdMember == DomEdge.MemberEnd() || !FromIdMember->value.IsUint()) {
			ErrorMessage = "Object should have \"from_id\" integer number.";
			return false;
		}
		const uint32_t FromVertexCustomId = FromIdMember->value.GetUint();
		const auto FromVertexId = Graph.VerticesCustomIdToEntityId.Find(FromVertexCustomId);
		if (!FromVertexId) {
			ErrorMessage = "Vertex with \"id\" " + FString::FromInt(FromVertexCustomId) + "not found.";
			return false;
		}
		NewEdge.ConnectedVertices[0] = *FromVertexId;
	}

	// to vertex id
	{
		const auto &ToIdMember = DomEdge.FindMember("to_id");
		if (ToIdMember == DomEdge.MemberEnd() || !ToIdMember->value.IsUint()) {
			ErrorMessage = "Object should have \"to_id\" integer number.";
			return false;
		}
		const uint32_t ToVertexCustomId = ToIdMember->value.GetUint();
		const auto ToVertexId = Graph.VerticesCustomIdToEntityId.Find(ToVertexCustomId);
		if (!ToVertexId) {
			ErrorMessage = "Vertex with \"id\" " + FString::FromInt(ToVertexCustomId) + "not found.";
			return false;
		}
		NewEdge.ConnectedVertices[1] = *ToVertexId;
	}

	// check if edge is not unique
	if (Graph.EdgesHashes.Contains(Const::ComputeHash(NewEdge, false))
		|| Graph.EdgesHashes.Contains(Const::ComputeHash(NewEdge, true)))
	{
		ErrorMessage = "Object is not unique.";
		return false;
	}

	Create(GraphId, NewEdge.ConnectedVertices[0], NewEdge.ConnectedVertices[1]);
	return true;
}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Const::Serialize"), STAT_EdgeCommands_Const_Serialize, STATGROUP_GRAPHS_PERF_COMMANDS);
void EdgeCommands::Const::Serialize(const EntityId EdgeId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Const_Serialize);

	const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);
	Writer.StartObject();

	// from vertex custom id
	Writer.Key("from_id");
	Writer.Uint(ES::GetEntity<VertexEntity>(Edge.ConnectedVertices[0]).CustomId);

	// to vertex custom id
	Writer.Key("to_id");
	Writer.Uint(ES::GetEntity<VertexEntity>(Edge.ConnectedVertices[1]).CustomId);

	Writer.EndObject();
}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Const::ComputeHash"), STAT_EdgeCommands_Const_ComputeHash, STATGROUP_GRAPHS_PERF_COMMANDS);
uint32_t EdgeCommands::Const::ComputeHash(const EdgeEntity &Edge, const bool ReverseVerticesIds) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Const_ComputeHash);

	const auto FromVertexId = Edge.ConnectedVertices[0];
	const auto ToVertexId = Edge.ConnectedVertices[1];

	if (!ReverseVerticesIds)
		return Utils::CantorPair(EntityId::Hash(FromVertexId), EntityId::Hash(ToVertexId));

	return Utils::CantorPair(EntityId::Hash(ToVertexId), EntityId::Hash(FromVertexId));
}
