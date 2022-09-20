#include "VertexCommands.h"
#include "EdgeCommands.h"
#include "GraphCommands.h"

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Create"), STAT_VertexCommands_Create, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::Create::Create(
	const EntityId GraphId, EntityId *NewVertexId,
	const uint32_t UserId,
	const FVector &Position,
	const FColor &Color
) : GraphsRendererCommand([GraphId, NewVertexId, UserId, &Position, &Color] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Create);

	const auto VertexId = ESMut().NewEntity<VertexEntity>();
	auto &Vertex = ESMut().GetEntityMut<VertexEntity>(VertexId);

	Vertex.GraphId = GraphId;
	Vertex.IsHit = false;
	Vertex.OverrideColor = ColorConsts::OverrideColorNone;

	Vertex.UserId = UserId;
	Vertex.Position = Position;
	Vertex.Color = Color;

	auto &ParentGraph = ESMut().GetEntityMut<GraphEntity>(GraphId);
	check(!ParentGraph.VertexUserIdToEntityId.Contains(UserId));
	ParentGraph.VertexUserIdToEntityId.Add(UserId, VertexId);
	check(!ParentGraph.VerticesIds.Contains(VertexId));
	ParentGraph.VerticesIds.Push(VertexId);

	if (NewVertexId)
		*NewVertexId = VertexId;

	MarkRendererComponentDirty(Renderer, {VERTEX, true, true});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Remove"), STAT_VertexCommands_Remove, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::Remove::Remove(const EntityId VertexId) : GraphsRendererCommand([VertexId] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Remove);

	auto &Vertex = ESMut().GetEntityMut<VertexEntity>(VertexId);
	auto &Graph = ESMut().GetEntityMut<GraphEntity>(Vertex.GraphId);

	if (Graph.VerticesIds.Num() > 1) {
		// remove from associated parent graph
		auto CheckNum = Graph.VerticesIds.Remove(VertexId);
		check(CheckNum == 1);

		CheckNum = Graph.VertexUserIdToEntityId.Remove(Vertex.UserId);
		check(CheckNum == 1);

		// move connected edges ids because otherwise the next
		// EdgeCommands::Remove command will invalidate EdgesIds while we iterating it
		const auto EdgeIds(MoveTemp(Vertex.EdgesIds));
		for (const auto EdgeId : EdgeIds)
			Renderer.ExecuteCommand(EdgeCommands::Remove(EdgeId));

		ESMut().RemoveEntity<VertexEntity>(VertexId);
		MarkRendererComponentDirty(Renderer, {VERTEX, true, true});
		return true;
	}

	// if we got here, then we are removing last vertex from graph
	// it means that we don't have edges in given graph
	// so we affects only vertices in command result
	Renderer.ExecuteCommand(GraphCommands::Remove(Vertex.GraphId));
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Reserve"), STAT_VertexCommands_Reserve, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::Reserve::Reserve(
	const EntityId GraphId,
	const uint32_t NewVerticesNum
) : GraphsRendererCommand([GraphId, NewVerticesNum] (AGraphsRenderer&) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Reserve);
	ESMut().ReserveForNewEntities<VertexEntity>(NewVerticesNum);

	auto &Graph = ESMut().GetEntityMut<GraphEntity>(GraphId);
	Graph.VertexUserIdToEntityId.Reserve(Graph.VertexUserIdToEntityId.Num() + NewVerticesNum);
	Graph.VerticesIds.Reserve(Graph.VerticesIds.Num() + NewVerticesNum);

	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::SetHit"), STAT_VertexCommands_SetHit, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::SetHit::SetHit(
	const EntityId VertexId,
	const bool IsHit
) : GraphsRendererCommand([VertexId, IsHit] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_SetHit);
	auto &Vertex = ESMut().GetEntityMut<VertexEntity>(VertexId);

	if (Vertex.IsHit == IsHit)
		return false;
	Vertex.IsHit = IsHit;

	MarkRendererComponentDirty(Renderer, {VERTEX, true, false});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::SetOverrideColor"), STAT_VertexCommands_SetOverrideColor, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::SetOverrideColor::SetOverrideColor(
	const EntityId VertexId,
	const FColor &OverrideColor
) : GraphsRendererCommand([VertexId, OverrideColor] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_SetOverrideColor);
	auto &Vertex = ESMut().GetEntityMut<VertexEntity>(VertexId);

	if (Vertex.OverrideColor == OverrideColor)
		return false;
	Vertex.OverrideColor = OverrideColor;

	MarkRendererComponentDirty(Renderer, {VERTEX, true, false});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Move"), STAT_VertexCommands_Move, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::Move::Move(
	const EntityId VertexId,
	const FVector &Delta
) : GraphsRendererCommand([VertexId, &Delta] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Move);
	auto &Vertex = ESMut().GetEntityMut<VertexEntity>(VertexId);

	Vertex.Position += Delta;

	MarkRendererComponentDirty(Renderer, {VERTEX, true, false});
	if (Vertex.EdgesIds.Num() > 0)
		MarkRendererComponentDirty(Renderer, {EDGE, true, false});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::ConstFuncs::Serialize"), STAT_VertexCommands_ConstFuncs_Serialize, STATGROUP_GRAPHS_PERF_COMMANDS);
void VertexCommands::ConstFuncs::Serialize(
	const EntityId VertexId,
	rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_ConstFuncs_Serialize);

	const auto &Vertex = ES::GetEntity<VertexEntity>(VertexId);
	Writer.StartObject();

	Writer.Key("id");
	Writer.Uint(Vertex.UserId);

	const auto PositionStr = Vertex.Position.ToString();
	const FTCHARToUTF8 PositionStrUTF(*PositionStr);
	Writer.Key("position");
	Writer.String(PositionStrUTF.Get(), PositionStrUTF.Length());

	auto ColorStr = "#" + Vertex.Color.ToHex();
	ColorStr.RemoveAt(ColorStr.Len() - 2, 2, false); // removes alpha channel
	const FTCHARToUTF8 ColorStrUTF(*ColorStr);
	Writer.Key("color");
	Writer.String(ColorStrUTF.Get(), ColorStrUTF.Length());

	Writer.EndObject();
}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::ConstFuncs::Deserialize"), STAT_VertexCommands_ConstFuncs_Deserialize, STATGROUP_GRAPHS_PERF_COMMANDS);
bool VertexCommands::ConstFuncs::Deserialize(
	const EntityId GraphId,
	const rapidjson::Value &DomVertex,
	FString &ErrorMessage,
	VertexEntity &NewVertex
) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_ConstFuncs_Deserialize);

	if (!DomVertex.IsObject()) {
		ErrorMessage = "Vertex error: Should be an object.";
		return false;
	}

	const auto &ParentGraph = ES::GetEntity<GraphEntity>(GraphId);

	const auto &IdMember = DomVertex.FindMember("id");
	bool IsIdOk = IdMember != DomVertex.MemberEnd() && IdMember->value.IsUint();
	if (IsIdOk) {
		NewVertex.UserId = IdMember->value.GetUint();
		IsIdOk = !ParentGraph.VertexUserIdToEntityId.Contains(NewVertex.UserId);
	}
	if (!IsIdOk) {
		ErrorMessage = "Vertex error: Object should have \"id\" unique integer number.";
		return false;
	}

	const auto &PositionMember = DomVertex.FindMember("position");
	bool IsPosOk = PositionMember != DomVertex.MemberEnd() && PositionMember->value.IsString();
	if (IsPosOk)
		IsPosOk = NewVertex.Position.InitFromString(FString(PositionMember->value.GetString()));
	if (!IsPosOk) {
		ErrorMessage = "Vertex #" + FString::FromInt(NewVertex.UserId)
			+ " error: Object should have \"position\" string in \"X=%3.3f Y=%3.3f Z=%3.3f\" format.";
		return false;
	}

	const auto &ColorMember = DomVertex.FindMember("color");
	bool IsColorOk = ColorMember != DomVertex.MemberEnd() && ColorMember->value.IsString();
	if (IsColorOk) {
		NewVertex.Color = FColor::FromHex(FString(ColorMember->value.GetString()));
		IsColorOk = NewVertex.Color != FColor::Transparent;
	}
	if (!IsColorOk) {
		ErrorMessage = "Vertex #" + FString::FromInt(NewVertex.UserId)
			+ " error: Object should have \"color\" string in #RRGGBB format.";
		return false;
	}

	return true;
}
