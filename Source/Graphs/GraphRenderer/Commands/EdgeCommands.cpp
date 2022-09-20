#include "EdgeCommands.h"
#include "VertexCommands.h"

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Create"), STAT_EdgeCommands_Create, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Create::Create(
	const EntityId GraphId, EntityId *NewEdgeId,
	const EntityId FromVertexId, const EntityId ToVertexId
) : GraphsRendererCommand([GraphId, NewEdgeId, FromVertexId, ToVertexId] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Create);
	return CreateImpl(Renderer, GraphId, NewEdgeId, FromVertexId, ToVertexId);
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::CreateWithUserIds"), STAT_EdgeCommands_CreateWithUserIds, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Create::Create(
	const EntityId GraphId, EntityId *NewEdgeId,
	const uint32_t FromVertexUserId, const uint32_t ToVertexUserId
) : GraphsRendererCommand([GraphId, NewEdgeId, FromVertexUserId, ToVertexUserId] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_CreateWithUserIds);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);
	return CreateImpl(
		Renderer,
		GraphId, NewEdgeId,
		Graph.VertexUserIdToEntityId.FindChecked(FromVertexUserId),
		Graph.VertexUserIdToEntityId.FindChecked(ToVertexUserId)
	);
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::CreateImpl"), STAT_EdgeCommands_CreateImpl, STATGROUP_GRAPHS_PERF_COMMANDS);
bool EdgeCommands::Create::CreateImpl(
	AGraphsRenderer &Renderer,
	const EntityId GraphId, EntityId* NewEdgeId,
	const EntityId FromVertexId, const EntityId ToVertexId
) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_CreateImpl);

	const auto EdgeId = ESMut().NewEntity<EdgeEntity>();
	auto &Edge = ESMut().GetEntityMut<EdgeEntity>(EdgeId);

	Edge.GraphId = GraphId;
	Edge.IsHit = false;
	Edge.OverrideColor = ColorConsts::OverrideColorNone;

	Edge.VerticesIds[0] = FromVertexId;
	Edge.VerticesIds[1] = ToVertexId;

	bool AlreadyInSet = false;
	auto &ParentGraph = ESMut().GetEntityMut<GraphEntity>(GraphId);

	check(!ParentGraph.EdgesIds.Contains(EdgeId));
	ParentGraph.EdgesIds.Push(EdgeId);

	const auto EdgeHash = ConstFuncs::ComputeHash(Edge, false);
	ParentGraph.EdgeHashes.Add(EdgeHash, &AlreadyInSet);
	check(!AlreadyInSet);
	if (!ParentGraph.IsOriented) {
		check(!ParentGraph.EdgeHashes.Contains(ConstFuncs::ComputeHash(Edge, true)));
	}

	ESMut().GetEntityMut<VertexEntity>(FromVertexId).EdgesIds.Add(EdgeId, &AlreadyInSet);
	check(!AlreadyInSet);

	ESMut().GetEntityMut<VertexEntity>(ToVertexId).EdgesIds.Add(EdgeId, &AlreadyInSet);
	check(!AlreadyInSet);

	if (NewEdgeId)
		*NewEdgeId = EdgeId;

	MarkRendererComponentDirty(Renderer, {EDGE, true, true});
	return true;
}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Remove"), STAT_EdgeCommands_Remove, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Remove::Remove(const EntityId EdgeId) : GraphsRendererCommand([EdgeId] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Remove);
	const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);

	// remove from associated parent graph
	auto &Graph = ESMut().GetEntityMut<GraphEntity>(Edge.GraphId);
	auto CheckNum = Graph.EdgesIds.Remove(EdgeId);
	check(CheckNum == 1);
	CheckNum = Graph.EdgeHashes.Remove(ConstFuncs::ComputeHash(Edge, false));
	check(CheckNum == 1);

	// remove connected edges
	for (const auto &VertexId : Edge.VerticesIds)
		ESMut().GetEntityMut<VertexEntity>(VertexId).EdgesIds.Remove(EdgeId);

	ESMut().RemoveEntity<EdgeEntity>(EdgeId);

	MarkRendererComponentDirty(Renderer, {EDGE, true, true});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Reserve"), STAT_EdgeCommands_Reserve, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Reserve::Reserve(
	const EntityId GraphId,
	const uint32_t NewEdgesNum
) : GraphsRendererCommand([GraphId, NewEdgesNum] (AGraphsRenderer&) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Reserve);
	ESMut().ReserveForNewEntities<EdgeEntity>(NewEdgesNum);

	auto &Graph = ESMut().GetEntityMut<GraphEntity>(GraphId);
	Graph.EdgesIds.Reserve(Graph.EdgesIds.Num() + NewEdgesNum);
	Graph.EdgeHashes.Reserve(Graph.EdgeHashes.Num() + NewEdgesNum);

	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::SetHit"), STAT_EdgeCommands_SetHit, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::SetHit::SetHit(
	const EntityId EdgeId,
	const bool IsHit
) : GraphsRendererCommand([EdgeId, IsHit] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_SetHit);
	auto &Edge = ESMut().GetEntityMut<EdgeEntity>(EdgeId);

	if (Edge.IsHit == IsHit)
		return false;
	Edge.IsHit = IsHit;

	MarkRendererComponentDirty(Renderer, {EDGE, true, false});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::SetOverrideColor"), STAT_EdgeCommands_SetOverrideColor, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::SetOverrideColor::SetOverrideColor(
	const EntityId EdgeId,
	const FColor &OverrideColor
) : GraphsRendererCommand([EdgeId, &OverrideColor] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_SetOverrideColor);
	auto &Edge = ESMut().GetEntityMut<EdgeEntity>(EdgeId);

	if (Edge.OverrideColor == OverrideColor)
		return false;
	Edge.OverrideColor = OverrideColor;

	MarkRendererComponentDirty(Renderer, {EDGE, true, false});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Move"), STAT_EdgeCommands_Move, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Move::Move(
	const EntityId EdgeId,
	const FVector &Delta
) : GraphsRendererCommand([EdgeId, &Delta] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Move);
	const auto &Edge = ESMut().GetEntity<EdgeEntity>(EdgeId);

	for (const auto &VertexId : Edge.VerticesIds)
		Renderer.ExecuteCommand(VertexCommands::Move(VertexId, Delta));

	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::ConstFuncs::Serialize"), STAT_EdgeCommands_ConstFuncs_Serialize, STATGROUP_GRAPHS_PERF_COMMANDS);
void EdgeCommands::ConstFuncs::Serialize(
	const EntityId EdgeId,
	rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_ConstFuncs_Serialize);

	const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);
	const auto &FromVertex = ES::GetEntity<VertexEntity>(Edge.VerticesIds[0]);
	const auto &ToVertex = ES::GetEntity<VertexEntity>(Edge.VerticesIds[1]);

	Writer.StartObject();

	Writer.Key("from_id");
	Writer.Uint(FromVertex.UserId);

	Writer.Key("to_id");
	Writer.Uint(ToVertex.UserId);

	Writer.EndObject();
}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::ConstFuncs::Deserialize"), STAT_EdgeCommands_ConstFuncs_Deserialize, STATGROUP_GRAPHS_PERF_COMMANDS);
bool EdgeCommands::ConstFuncs::Deserialize(
	const EntityId GraphId,
	const rapidjson::Value &DomEdge,
	FString &ErrorMessage,
	EdgeEntity &NewEdge
) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_ConstFuncs_Deserialize);

	if (!DomEdge.IsObject()) {
		ErrorMessage = "Edge error: Should be an object.";
		return false;
	}

	const auto &ParentGraph = ES::GetEntity<GraphEntity>(GraphId);

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
		EntityId::Hash(Edge.VerticesIds[!ReverseVerticesIds ? 0 : 1]),
		EntityId::Hash(Edge.VerticesIds[!ReverseVerticesIds ? 1 : 0])
	);
}
