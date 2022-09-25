#include "EdgeCommands.h"
#include "VertexCommands.h"
#include "Algo/IsSorted.h"

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Create"), STAT_EdgeCommands_Create, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Create::Create(
	const EntityId GraphId, EntityId *NewEdgeId,
	const EntityId FromVertexId, const EntityId ToVertexId
) : GraphsRenderersCommand([=] {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Create);

	const auto EdgeId = GetESMut().NewEntity<EdgeEntity>();
	auto &Edge = GetESMut().GetEntityMut<EdgeEntity>(EdgeId);

	Edge.GraphId = GraphId;
	Edge.IsHit = false;
	Edge.OverrideColor = ColorConsts::OverrideColorNone;

	Edge.ConnectedVertices[0] = FromVertexId;
	Edge.ConnectedVertices[1] = ToVertexId;

	auto &Graph = GetESMut().GetEntityMut<GraphEntity>(GraphId);

	bool AlreadyInSet = false;
	const auto EdgeHash = Consts::ComputeHash(Edge, false);
	Graph.EdgesHashes.Add(EdgeHash, &AlreadyInSet);
	check(!AlreadyInSet);
	check(!Graph.EdgesHashes.Contains(Consts::ComputeHash(Edge, true)));

	check(Algo::IsSorted(Graph.Edges));
	const auto LB = Algo::LowerBound(Graph.Edges, EdgeId);
	check(LB == Graph.Edges.Num() || Graph.Edges[LB] != EdgeId);
	Graph.Edges.Insert(EdgeId, LB);

	GetESMut().GetEntityMut<VertexEntity>(FromVertexId).ConnectedEdges.Add(EdgeId, &AlreadyInSet);
	check(!AlreadyInSet);

	GetESMut().GetEntityMut<VertexEntity>(ToVertexId).ConnectedEdges.Add(EdgeId, &AlreadyInSet);
	check(!AlreadyInSet);

	if (NewEdgeId)
		*NewEdgeId = EdgeId;

	GetGraphRenderer(GraphId)->MarkDirty({EDGE, true, true});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Remove"), STAT_EdgeCommands_Remove, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Remove::Remove(const EntityId EdgeId) : GraphsRenderersCommand([=] {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Remove);
	const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);

	// remove from associated parent graph
	auto &Graph = GetESMut().GetEntityMut<GraphEntity>(Edge.GraphId);
	check(Algo::IsSorted(Graph.Edges));
	const auto IdxToRemove = Algo::BinarySearch(Graph.Edges, EdgeId);
	check(IdxToRemove != INDEX_NONE);
	Graph.Edges.RemoveAt(IdxToRemove, 1, false);

	const auto CheckNum = Graph.EdgesHashes.Remove(Consts::ComputeHash(Edge, false));
	check(CheckNum == 1);
	check(!Graph.EdgesHashes.Contains(Consts::ComputeHash(Edge, true)));

	// remove connected edges
	for (const auto VertexId : Edge.ConnectedVertices)
		GetESMut().GetEntityMut<VertexEntity>(VertexId).ConnectedEdges.Remove(EdgeId);

	const auto GraphId = Edge.GraphId;
	GetESMut().RemoveEntity<EdgeEntity>(EdgeId);
	GetGraphRenderer(GraphId)->MarkDirty({EDGE, true, true});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Reserve"), STAT_EdgeCommands_Reserve, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Reserve::Reserve(const EntityId GraphId, const uint32_t NewEdgesNum) : GraphsRenderersCommand([=] {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Reserve);
	GetESMut().Reserve<EdgeEntity>(NewEdgesNum);

	auto &Graph = GetESMut().GetEntityMut<GraphEntity>(GraphId);
	Graph.EdgesHashes.Reserve(Graph.EdgesHashes.Num() + NewEdgesNum);
	Graph.Edges.Reserve(Graph.Edges.Num() + NewEdgesNum);

	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::SetHit"), STAT_EdgeCommands_SetHit, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::SetHit::SetHit(const EntityId EdgeId, const bool IsHit) : GraphsRenderersCommand([=] {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_SetHit);
	auto &Edge = GetESMut().GetEntityMut<EdgeEntity>(EdgeId);

	if (Edge.IsHit == IsHit)
		return false;
	Edge.IsHit = IsHit;

	GetGraphRenderer(Edge.GraphId)->MarkDirty({EDGE, true, false});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::SetOverrideColor"), STAT_EdgeCommands_SetOverrideColor, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::SetOverrideColor::SetOverrideColor(
	const EntityId EdgeId,
	const FColor &OverrideColor
) : GraphsRenderersCommand([=, &OverrideColor] {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_SetOverrideColor);
	auto &Edge = GetESMut().GetEntityMut<EdgeEntity>(EdgeId);

	if (Edge.OverrideColor == OverrideColor)
		return false;
	Edge.OverrideColor = OverrideColor;

	GetGraphRenderer(Edge.GraphId)->MarkDirty({EDGE, true, false});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Move"), STAT_EdgeCommands_Move, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Move::Move(const EntityId EdgeId, const FVector &Delta) : GraphsRenderersCommand([=, &Delta] {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Move);
	const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);

	ExecuteSubCommand(VertexCommands::Move(Edge.ConnectedVertices[0], Delta));
	ExecuteSubCommand(VertexCommands::Move(Edge.ConnectedVertices[1], Delta));

	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Consts::Serialize"), STAT_EdgeCommands_Consts_Serialize, STATGROUP_GRAPHS_PERF_COMMANDS);
void EdgeCommands::Consts::Serialize(const EntityId EdgeId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Consts_Serialize);

	const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);
	Writer.StartObject();

	// from vertex' custom id
	Writer.Key("from_id");
	Writer.Uint(ES::GetEntity<VertexEntity>(Edge.ConnectedVertices[0]).CustomId);

	// to vertex' custom id
	Writer.Key("to_id");
	Writer.Uint(ES::GetEntity<VertexEntity>(Edge.ConnectedVertices[1]).CustomId);

	Writer.EndObject();
}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Consts::Deserialize"), STAT_EdgeCommands_Consts_Deserialize, STATGROUP_GRAPHS_PERF_COMMANDS);
bool EdgeCommands::Consts::Deserialize(
	const rapidjson::Value &DomEdge,
	GraphImportData &GraphData,
	const TSet<uint32_t> &VerticesCustomIds,
	TSet<uint32_t> &ImportEdgesHashes,
	FString &ErrorMessage
) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Consts_Deserialize);

	if (!DomEdge.IsObject()) {
		ErrorMessage = "Should be an object.";
		return false;
	}

	EdgeImportData NewEdgeData;

	// from vertex' custom id
	{
		const auto &FromIdMember = DomEdge.FindMember("from_id");
		if (FromIdMember == DomEdge.MemberEnd() || !FromIdMember->value.IsUint()) {
			ErrorMessage = "Object should have \"from_id\" integer number.";
			return false;
		}
		NewEdgeData.ConnectedVertexCustomIds[0] = FromIdMember->value.GetUint();
		if (!VerticesCustomIds.Contains(NewEdgeData.ConnectedVertexCustomIds[0])) {
			ErrorMessage = "Vertex with \"id\" " + FString::FromInt(NewEdgeData.ConnectedVertexCustomIds[0]) + "not found.";
			return false;
		}
	}

	// to vertex' custom id
	{
		const auto &ToIdMember = DomEdge.FindMember("to_id");
		if (ToIdMember == DomEdge.MemberEnd() || !ToIdMember->value.IsUint()) {
			ErrorMessage = "Object should have \"to_id\" integer number.";
			return false;
		}
		NewEdgeData.ConnectedVertexCustomIds[1] = ToIdMember->value.GetUint();
		if (!VerticesCustomIds.Contains(NewEdgeData.ConnectedVertexCustomIds[1])) {
			ErrorMessage = "Vertex with \"id\" " + FString::FromInt(NewEdgeData.ConnectedVertexCustomIds[1]) + "not found.";
			return false;
		}
	}

	const auto Hash = Utils::CantorPair(
		NewEdgeData.ConnectedVertexCustomIds[0],
		NewEdgeData.ConnectedVertexCustomIds[1]
	);
	bool AlreadyInSet = false;
	ImportEdgesHashes.Add(Hash, &AlreadyInSet);
	if (AlreadyInSet) {
		ErrorMessage = "Object is not unique.";
		return false;
	}

	const auto ReversedHash = Utils::CantorPair(
		NewEdgeData.ConnectedVertexCustomIds[1],
		NewEdgeData.ConnectedVertexCustomIds[2]
	);
	if (ImportEdgesHashes.Contains(ReversedHash)) {
		ErrorMessage = "Object is not unique.";
		return false;
	}

	GraphData.Edges.Push(MoveTemp(NewEdgeData));
	return true;
}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Consts::ComputeHash"), STAT_EdgeCommands_Consts_ComputeHash, STATGROUP_GRAPHS_PERF_COMMANDS);
uint32_t EdgeCommands::Consts::ComputeHash(const EdgeEntity &Edge, const bool ReverseVerticesIds) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Consts_ComputeHash);

	const auto FromVertexId = Edge.ConnectedVertices[0];
	const auto ToVertexId = Edge.ConnectedVertices[1];

	if (!ReverseVerticesIds)
		return Utils::CantorPair(EntityId::Hash(FromVertexId), EntityId::Hash(ToVertexId));

	return Utils::CantorPair(EntityId::Hash(ToVertexId), EntityId::Hash(FromVertexId));
}
