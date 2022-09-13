#include "EdgeCommands.h"
#include "VertexCommands.h"

EdgeCommands::Create::Create(
	const EntityId &GraphId, EntityId *NewEdgeId,
	const EntityId &FromVertexId, const EntityId &ToVertexId
) : Command([&, NewEdgeId] (EntityStorage &Storage) -> bool {
	return CreateImpl(Storage, GraphId, NewEdgeId, FromVertexId, ToVertexId);
}) {}

EdgeCommands::Create::Create(
	const EntityId &GraphId, EntityId *NewEdgeId,
	const uint32_t FromVertexUserId, const uint32_t ToVertexUserId
) : Command([&, NewEdgeId, FromVertexUserId, ToVertexUserId] (EntityStorage &Storage) -> bool {
	auto &ParentGraph = Storage.GetEntityMut<GraphEntity>(GraphId);
	return CreateImpl(
		Storage,
		GraphId, NewEdgeId,
		ParentGraph.VertexUserIdToEntityId.FindChecked(FromVertexUserId),
		ParentGraph.VertexUserIdToEntityId.FindChecked(ToVertexUserId)
	);
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Create"), STAT_EdgeCommands_Create, STATGROUP_GRAPHS_PERF_COMMANDS);
bool EdgeCommands::Create::CreateImpl(
	EntityStorage &Storage,
	const EntityId& GraphId, EntityId* NewEdgeId,
	const EntityId &FromVertexId, const EntityId &ToVertexId
) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Create);

	const auto EdgeId = Storage.NewEntity<EdgeEntity>();
	auto &Edge = Storage.GetEntityMut<EdgeEntity>(EdgeId);

	Edge.GraphId = GraphId;
	Edge.IsHit = false;
	Edge.OverrideColor = ColorConsts::OverrideColorNone;

	Edge.VerticesIds[0] = FromVertexId;
	Edge.VerticesIds[1] = ToVertexId;

	bool AlreadyInSet = false;
	auto &ParentGraph = Storage.GetEntityMut<GraphEntity>(GraphId);

	check(!ParentGraph.EdgesIds.Contains(EdgeId));
	ParentGraph.EdgesIds.Push(EdgeId);

	const auto EdgeHash = ConstFuncs::ComputeHash(Edge, false);
	ParentGraph.EdgeHashes.Add(EdgeHash, &AlreadyInSet);
	check(!AlreadyInSet);
	if (!ParentGraph.IsOriented) {
		check(!ParentGraph.EdgeHashes.Contains(ConstFuncs::ComputeHash(Edge, true)));
	}

	Storage.GetEntityMut<VertexEntity>(FromVertexId).EdgesIds.Add(EdgeId, &AlreadyInSet);
	check(!AlreadyInSet);

	Storage.GetEntityMut<VertexEntity>(ToVertexId).EdgesIds.Add(EdgeId, &AlreadyInSet);
	check(!AlreadyInSet);

	if (NewEdgeId)
		*NewEdgeId = EdgeId;

	return true;
}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Remove"), STAT_EdgeCommands_Remove, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Remove::Remove(const EntityId &EdgeId) : Command([&] (EntityStorage &Storage) -> bool {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Remove);
	const auto &Edge = Storage.GetEntity<EdgeEntity>(EdgeId);

	// remove from associated parent graph
	auto &Graph = Storage.GetEntityMut<GraphEntity>(Edge.GraphId);
	auto CheckNum = Graph.EdgesIds.Remove(EdgeId);
	check(CheckNum == 1);
	CheckNum = Graph.EdgeHashes.Remove(ConstFuncs::ComputeHash(Edge, false));
	check(CheckNum == 1);

	// remove connected edges
	for (const auto &VertexId : Edge.VerticesIds)
		Storage.GetEntityMut<VertexEntity>(VertexId).EdgesIds.Remove(EdgeId);

	Storage.RemoveEntity<EdgeEntity>(EdgeId);
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Reserve"), STAT_EdgeCommands_Reserve, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Reserve::Reserve(
	const EntityId &GraphId,
	const uint32_t NewEdgesNum
) : Command([&, NewEdgesNum] (EntityStorage &Storage) -> bool {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Reserve);

	auto &EdgesStorage = Storage.GetStorageMut<EdgeEntity>().Data;
	EdgesStorage.Reserve(EdgesStorage.Num() + NewEdgesNum);

	auto &Graph = Storage.GetEntityMut<GraphEntity>(GraphId);
	Graph.EdgesIds.Reserve(Graph.EdgesIds.Num() + NewEdgesNum);
	Graph.EdgeHashes.Reserve(Graph.EdgeHashes.Num() + NewEdgesNum);

	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::SetHit"), STAT_EdgeCommands_SetHit, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::SetHit::SetHit(
	const EntityId &EdgeId,
	const bool IsHit
) : Command([&, IsHit] (EntityStorage &Storage) -> bool {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_SetHit);
	auto &Edge = Storage.GetEntityMut<EdgeEntity>(EdgeId);

	if (Edge.IsHit == IsHit)
		return false;
	Edge.IsHit = IsHit;

	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::SetOverrideColor"), STAT_EdgeCommands_SetOverrideColor, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::SetOverrideColor::SetOverrideColor(
	const EntityId &EdgeId,
	const FColor &OverrideColor
) : Command([&] (EntityStorage &Storage) -> bool {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_SetOverrideColor);
	auto &Edge = Storage.GetEntityMut<EdgeEntity>(EdgeId);

	if (Edge.OverrideColor == OverrideColor)
		return false;
	Edge.OverrideColor = OverrideColor;

	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Move"), STAT_EdgeCommands_Move, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Move::Move(
	const EntityId &EdgeId,
	const FVector &Delta
) : Command([&] (EntityStorage &Storage) -> bool {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Move);
	const auto &Edge = Storage.GetEntity<EdgeEntity>(EdgeId);

	for (const auto &VertexId : Edge.VerticesIds)
		ExecuteSubCommand(VertexCommands::Move(VertexId, Delta), Storage);

	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::ConstFuncs::Serialize"), STAT_EdgeCommands_ConstFuncs_Serialize, STATGROUP_GRAPHS_PERF_COMMANDS);
void EdgeCommands::ConstFuncs::Serialize(
	const EntityStorage &Storage,
	const EntityId &EdgeId,
	rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_ConstFuncs_Serialize);

	const auto &Edge = Storage.GetEntity<EdgeEntity>(EdgeId);
	const auto &FromVertex = Storage.GetEntity<VertexEntity>(Edge.VerticesIds[0]);
	const auto &ToVertex = Storage.GetEntity<VertexEntity>(Edge.VerticesIds[1]);

	Writer.StartObject();

	Writer.Key("from_id");
	Writer.Uint(FromVertex.UserId);

	Writer.Key("to_id");
	Writer.Uint(ToVertex.UserId);

	Writer.EndObject();
}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::ConstFuncs::Deserialize"), STAT_EdgeCommands_ConstFuncs_Deserialize, STATGROUP_GRAPHS_PERF_COMMANDS);
bool EdgeCommands::ConstFuncs::Deserialize(
	const EntityStorage &Storage,
	const EntityId &GraphId,
	const rapidjson::Value &DomEdge,
	FString &ErrorMessage,
	EdgeEntity &NewEdge
) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_ConstFuncs_Deserialize);

	if (!DomEdge.IsObject()) {
		ErrorMessage = "Edge error: Should be an object.";
		return false;
	}

	const auto &ParentGraph = Storage.GetEntity<GraphEntity>(GraphId);

	int i = 0;
	TStaticArray<uint32_t, 2> VertexUserIds;
	for (const auto Key : {"from_id", "to_id"}) {
		const auto &IdMember = DomEdge.FindMember(Key);
		if (IdMember == DomEdge.MemberEnd() || !IdMember->value.IsUint()) {
			ErrorMessage = "Edge error: Object should have \"" + FString(Key) + "\" integer number.";
			return false;
		}
		VertexUserIds[i] = IdMember->value.GetUint();
		const auto VertexId = ParentGraph.VertexUserIdToEntityId.Find(VertexUserIds[i]);
		if (!VertexId) {
			ErrorMessage = "Edge error: Vertex with ID " + FString::FromInt(VertexUserIds[i]) + " not found.";
			return false;
		}
		NewEdge.VerticesIds[i++] = *VertexId;
	}

	if (ParentGraph.EdgeHashes.Contains(ComputeHash(NewEdge, false))) {
		ErrorMessage = "Edge error: {"
			+ FString::FromInt(VertexUserIds[0])
			+ ", "
			+ FString::FromInt(VertexUserIds[1])
			+ "} declared multiple times.";
		return false;
	}
	if (!ParentGraph.IsOriented && ParentGraph.EdgeHashes.Contains(ComputeHash(NewEdge, true))) {
		ErrorMessage = "Edge error: {"
			+ FString::FromInt(VertexUserIds[0])
			+ ", "
			+ FString::FromInt(VertexUserIds[1])
			+ "} declared multiple times in non-oriented graph.";
		return false;
	}

	return true;
}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::ConstFuncs::ComputeHash"), STAT_EdgeCommands_ConstFuncs_ComputeHash, STATGROUP_GRAPHS_PERF_COMMANDS);
uint32_t EdgeCommands::ConstFuncs::ComputeHash(const EdgeEntity &Edge, const bool ReverseVerticesIds) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_ConstFuncs_ComputeHash);

	check(Edge.VerticesIds[0] != EntityId::NONE());
	check(Edge.VerticesIds[1] != EntityId::NONE());

	return Utils::CantorPair(
		EntityId::GetHash(Edge.VerticesIds[!ReverseVerticesIds ? 0 : 1]),
		EntityId::GetHash(Edge.VerticesIds[!ReverseVerticesIds ? 1 : 0])
	);
}
