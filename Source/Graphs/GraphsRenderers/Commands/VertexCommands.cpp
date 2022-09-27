#include "VertexCommands.h"
#include "EdgeCommands.h"
#include "GraphCommands.h"
#include "Algo/IsSorted.h"

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Create"), STAT_VertexCommands_Create, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::Create::Create(
	const EntityId GraphId, EntityId *NewVertexId,
	const uint32_t CustomVertexId,
	const FVector &Position,
	const FColor &Color
) : GraphsRenderersCommand([=, &Position, &Color] {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Create);

	const auto VertexId = GetESMut().NewEntity<VertexEntity>();
	auto &Vertex = GetESMut().GetEntityMut<VertexEntity>(VertexId);

	Vertex.GraphId = GraphId;
	Vertex.IsHit = false;
	Vertex.OverrideColor = ColorConsts::OverrideColorNone;

	Vertex.CustomId = CustomVertexId;
	Vertex.Position = Position;
	Vertex.Color = Color;

	auto &Graph = GetESMut().GetEntityMut<GraphEntity>(GraphId);

	check(!Graph.VerticesCustomIdToEntityId.Contains(CustomVertexId));
	Graph.VerticesCustomIdToEntityId.Add(CustomVertexId, VertexId);

	check(Algo::IsSorted(Graph.Vertices));
	const auto LB = Algo::LowerBound(Graph.Vertices, VertexId);
	check(LB == Graph.Vertices.Num() || Graph.Vertices[LB] != VertexId);
	Graph.Vertices.Insert(VertexId, LB);

	if (NewVertexId)
		*NewVertexId = VertexId;

	GetGraphRenderer(GraphId)->MarkDirty({VERTEX, true, true});
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Remove"), STAT_VertexCommands_Remove, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::Remove::Remove(const EntityId VertexId) : GraphsRenderersCommand([=] {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Remove);

	auto &Vertex = GetESMut().GetEntityMut<VertexEntity>(VertexId);
	auto &Graph = GetESMut().GetEntityMut<GraphEntity>(Vertex.GraphId);

	if (Graph.Vertices.Num() > 1) {
		// remove from associated parent graph
		check(Algo::IsSorted(Graph.Vertices));
		const auto IdxToRemove = Algo::BinarySearch(Graph.Vertices, VertexId);
		check(IdxToRemove != INDEX_NONE);
		Graph.Vertices.RemoveAt(IdxToRemove, 1, false);

		const auto CheckNum = Graph.VerticesCustomIdToEntityId.Remove(Vertex.CustomId);
		check(CheckNum == 1);

		// move connected edges ids because otherwise the next
		// EdgeCommands::Remove command will invalidate EdgesIds while we iterating it
		const auto EdgeIds(MoveTemp(Vertex.ConnectedEdges));
		for (const auto EdgeId : EdgeIds)
			ExecuteSubCommand(EdgeCommands::Remove(EdgeId));

		const auto GraphId = Vertex.GraphId;
		GetESMut().RemoveEntity<VertexEntity>(VertexId);
		GetGraphRenderer(GraphId)->MarkDirty({VERTEX, true, true});
		return;
	}

	// if we got here, then we are removing last vertex from graph
	ExecuteSubCommand(GraphCommands::Remove(Vertex.GraphId));
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Reserve"), STAT_VertexCommands_Reserve, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::Reserve::Reserve(const EntityId GraphId, const uint32_t NewVerticesNum) : GraphsRenderersCommand([=] {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Reserve);
	GetESMut().Reserve<VertexEntity>(NewVerticesNum);

	auto &Graph = GetESMut().GetEntityMut<GraphEntity>(GraphId);
	Graph.VerticesCustomIdToEntityId.Reserve(Graph.VerticesCustomIdToEntityId.Num() + NewVerticesNum);
	Graph.Vertices.Reserve(Graph.Vertices.Num() + NewVerticesNum);
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::SetHit"), STAT_VertexCommands_SetHit, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::SetHit::SetHit(
	const EntityId VertexId,
	const bool IsHit
) : GraphsRenderersCommand([=] {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_SetHit);
	auto &Vertex = GetESMut().GetEntityMut<VertexEntity>(VertexId);

	if (Vertex.IsHit == IsHit)
		return;
	Vertex.IsHit = IsHit;

	GetGraphRenderer(Vertex.GraphId)->MarkDirty({VERTEX, true, false});
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::SetOverrideColor"), STAT_VertexCommands_SetOverrideColor, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::SetOverrideColor::SetOverrideColor(
	const EntityId VertexId,
	const FColor &OverrideColor
) : GraphsRenderersCommand([=, &OverrideColor] {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_SetOverrideColor);
	auto &Vertex = GetESMut().GetEntityMut<VertexEntity>(VertexId);

	if (Vertex.OverrideColor == OverrideColor)
		return;
	Vertex.OverrideColor = OverrideColor;

	GetGraphRenderer(Vertex.GraphId)->MarkDirty({VERTEX, true, false});
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Move"), STAT_VertexCommands_Move, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::Move::Move(
	const EntityId VertexId,
	const FVector &Delta
) : GraphsRenderersCommand([=, &Delta] {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Move);

	auto &Vertex = GetESMut().GetEntityMut<VertexEntity>(VertexId);
	Vertex.Position += Delta;

	const auto GraphRenderer = GetGraphRenderer(Vertex.GraphId);
	GraphRenderer->MarkDirty({VERTEX, true, false});
	if (Vertex.ConnectedEdges.Num() > 0)
		GraphRenderer->MarkDirty({EDGE, true, false});
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Consts::Serialize"), STAT_VertexCommands_Consts_Serialize, STATGROUP_GRAPHS_PERF_COMMANDS);
void VertexCommands::Consts::Serialize(
	const EntityId VertexId,
	rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Consts_Serialize);

	const auto &Vertex = ES::GetEntity<VertexEntity>(VertexId);
	Writer.StartObject();

	// custom id
	{
		Writer.Key("id");
		Writer.Uint(Vertex.CustomId);
	}

	// position in "X=%3.3f Y=%3.3f Z=%3.3f" format
	{
		const auto PositionStr = Vertex.Position.ToString();
		const FTCHARToUTF8 PositionStrUTF(*PositionStr);
		Writer.Key("position");
		Writer.String(PositionStrUTF.Get(), PositionStrUTF.Length());
	}

	// color in #RRGGBB format
	{
		auto ColorStr = "#" + Vertex.Color.ToHex();
		ColorStr.RemoveAt(ColorStr.Len() - 2, 2, false); // removes alpha channel
		const FTCHARToUTF8 ColorStrUTF(*ColorStr);
		Writer.Key("color");
		Writer.String(ColorStrUTF.Get(), ColorStrUTF.Length());
	}

	Writer.EndObject();
}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Consts::Deserialize"), STAT_VertexCommands_Consts_Deserialize, STATGROUP_GRAPHS_PERF_COMMANDS);
bool VertexCommands::Consts::Deserialize(
	const rapidjson::Value &DomVertex,
	GraphImportData &GraphData,
	TSet<uint32_t> &VerticesCustomIds,
	FString &ErrorMessage
) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Consts_Deserialize);

	if (!DomVertex.IsObject()) {
		ErrorMessage = "Should be an object.";
		return false;
	}

	VertexImportData NewVertexData;

	// custom id
	{
		const auto &IdMember = DomVertex.FindMember("id");
		if (IdMember == DomVertex.MemberEnd() || !IdMember->value.IsUint()) {
			ErrorMessage = "Object should have \"id\" unique integer number.";
			return false;
		}
		NewVertexData.CustomId = IdMember->value.GetUint();
		if (VerticesCustomIds.Contains(NewVertexData.CustomId)) {
			ErrorMessage = "\"id\" (" + FString::FromInt(NewVertexData.CustomId) + ") is not unique.";
			return false;
		}
	}

	// position
	{
		const auto &PositionMember = DomVertex.FindMember("position");
		if (PositionMember == DomVertex.MemberEnd() || !PositionMember->value.IsString()) {
			ErrorMessage = "Object should have \"position\" string.";
			return false;
		}
		if (!NewVertexData.Position.InitFromString(FString(PositionMember->value.GetString()))) {
			ErrorMessage = "\"position\" should be in \"X=%3.3f Y=%3.3f Z=%3.3f\" format.";
			return false;
		}
	}

	// color
	{
		const auto &ColorMember = DomVertex.FindMember("color");
		if (ColorMember == DomVertex.MemberEnd() || !ColorMember->value.IsString()) {
			ErrorMessage = "Object should have \"color\" string.";
			return false;
		}
		NewVertexData.Color = FColor::FromHex(FString(ColorMember->value.GetString()));
		if (NewVertexData.Color == FColor(ForceInitToZero)) {
			ErrorMessage = "\"color\" shoud be in #RRGGBB format.";
			return false;
		}
	}

	bool AlreadyInSet = false;
	VerticesCustomIds.Add(NewVertexData.CustomId, &AlreadyInSet);
	check(!AlreadyInSet);

	GraphData.Vertices.Push(MoveTemp(NewVertexData));
	return true;
}
