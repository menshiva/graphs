#include "EdgeCommands.h"
#include "VertexCommands.h"

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Mutable::Create"), STAT_EdgeCommands_Mutable_Create, STATGROUP_GRAPHS_PERF_COMMANDS);
EntityId EdgeCommands::Mutable::Create(
	const EntityId GraphId,
	const EntityId FromVertexId, const EntityId ToVertexId,
	const float Weight
) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Mutable_Create);

	// create new edge entity
	const auto EdgeId = ES::NewEntity<EdgeEntity>();

	// fill new entity with default and given properties
	auto &Edge = ES::GetEntityMut<EdgeEntity>(EdgeId);
	Edge.GraphId = GraphId;
	Edge.IsHit = false;
	Edge.OverrideColor = ColorConsts::OverrideColorNone;
	Edge.ConnectedVertices[0] = FromVertexId;
	Edge.ConnectedVertices[1] = ToVertexId;
	Edge.Weight = Weight;

	// add new entity to parent graph
	auto &Graph = ES::GetEntityMut<GraphEntity>(GraphId);
	bool AlreadyInSet = false;
	// store its hash so that we can fastly determine if undirected (FromVertexId, ToVertexId) edge is already in the graph
	Graph.EdgesHashes.Add(Const::ComputeHash(Edge, false), &AlreadyInSet);
	check(!AlreadyInSet);
	check(!Graph.EdgesHashes.Contains(Const::ComputeHash(Edge, true)));
	Graph.Edges.Add(EdgeId, &AlreadyInSet);
	check(!AlreadyInSet);

	// add to connected vertices
	auto &FirstVertex = ES::GetEntityMut<VertexEntity>(FromVertexId);
	auto &SecondVertex = ES::GetEntityMut<VertexEntity>(ToVertexId);
	check(FirstVertex.GraphId == SecondVertex.GraphId); // check so that the 2 given vertices are from the same graph
	FirstVertex.ConnectedEdges.Add(EdgeId, &AlreadyInSet);
	check(!AlreadyInSet);
	SecondVertex.ConnectedEdges.Add(EdgeId, &AlreadyInSet);
	check(!AlreadyInSet);

	return EdgeId;
}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Mutable::Remove"), STAT_EdgeCommands_Mutable_Remove, STATGROUP_GRAPHS_PERF_COMMANDS);
void EdgeCommands::Mutable::Remove(const EntityId EdgeId) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Mutable_Remove);
	const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);

	// remove from the associated parent graph
	auto &Graph = ES::GetEntityMut<GraphEntity>(Edge.GraphId);
	auto CheckNum = Graph.EdgesHashes.Remove(Const::ComputeHash(Edge, false));
	check(CheckNum == 1);
	check(!Graph.EdgesHashes.Contains(Const::ComputeHash(Edge, true)));
	CheckNum = Graph.Edges.Remove(EdgeId);
	check(CheckNum == 1);

	// remove from the connected vertices
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
		ErrorMessage = "Should be a JSON object.";
		return false;
	}

	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);
	EdgeEntity NewEdge;

	// from vertex
	{
		const auto &FromMember = DomEdge.FindMember("from");
		if (FromMember == DomEdge.MemberEnd() || !FromMember->value.IsUint()) {
			ErrorMessage = "Object's \"from\" attribute should exist and have an integer number.";
			return false;
		}
		const uint32_t FromVertexLabel = FromMember->value.GetUint();
		const auto FromVertexId = Graph.VerticesLabelToEntityId.Find(FromVertexLabel);
		if (!FromVertexId) {
			ErrorMessage = "Vertex with \"label\" " + FString::FromInt(FromVertexLabel) + "not found.";
			return false;
		}
		NewEdge.ConnectedVertices[0] = *FromVertexId;
	}

	// to vertex id
	{
		const auto &ToMember = DomEdge.FindMember("to");
		if (ToMember == DomEdge.MemberEnd() || !ToMember->value.IsUint()) {
			ErrorMessage = "Object's \"to\" attribute should exist and have an integer number.";
			return false;
		}
		const uint32_t ToVertexLabel = ToMember->value.GetUint();
		const auto ToVertexId = Graph.VerticesLabelToEntityId.Find(ToVertexLabel);
		if (!ToVertexId) {
			ErrorMessage = "Vertex with \"label\" " + FString::FromInt(ToVertexLabel) + "not found.";
			return false;
		}
		NewEdge.ConnectedVertices[1] = *ToVertexId;
	}

	// weight
	{
		const auto &WeightMember = DomEdge.FindMember("weight");
		if (WeightMember != DomEdge.MemberEnd()) {
			if (!WeightMember->value.IsFloat()) {
				ErrorMessage = "Object's \"weight\" value should be a floating-point number.";
				return false;
			}
			NewEdge.Weight = WeightMember->value.GetFloat();
		}
	}

	// check if edge is unique
	if (Graph.EdgesHashes.Contains(Const::ComputeHash(NewEdge, false))
		|| Graph.EdgesHashes.Contains(Const::ComputeHash(NewEdge, true)))
	{
		ErrorMessage = "Object is not unique.";
		return false;
	}

	Create(GraphId, NewEdge.ConnectedVertices[0], NewEdge.ConnectedVertices[1], NewEdge.Weight);
	return true;
}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Const::Serialize"), STAT_EdgeCommands_Const_Serialize, STATGROUP_GRAPHS_PERF_COMMANDS);
void EdgeCommands::Const::Serialize(const EntityId EdgeId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Const_Serialize);

	const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);
	Writer.StartObject();

	// from vertex custom id
	Writer.Key("from");
	Writer.Uint(ES::GetEntity<VertexEntity>(Edge.ConnectedVertices[0]).Label);

	// to vertex custom id
	Writer.Key("to");
	Writer.Uint(ES::GetEntity<VertexEntity>(Edge.ConnectedVertices[1]).Label);

	// weight
	if (Edge.Weight != 0.0f) {
		Writer.Key("weight");
		Writer.Double(Edge.Weight);
	}

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
