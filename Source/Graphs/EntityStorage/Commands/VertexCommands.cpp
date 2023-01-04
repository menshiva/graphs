#include "VertexCommands.h"
#include "EdgeCommands.h"
#include "GraphCommands.h"

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Mutable::Create"), STAT_VertexCommands_Mutable_Create, STATGROUP_GRAPHS_PERF_COMMANDS);
EntityId VertexCommands::Mutable::Create(
	const EntityId GraphId,
	const uint32_t Label, const FVector &Position, const FColor &Color
) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Mutable_Create);

	// create a new vertex entity
	const auto VertexId = ES::NewEntity<VertexEntity>();

	// fill the new entity with given properties
	auto &Vertex = ES::GetEntityMut<VertexEntity>(VertexId);
	Vertex.GraphId = GraphId;
	Vertex.IsHit = false;
	Vertex.OverrideColor = ColorConsts::OverrideColorNone;
	Vertex.Label = Label;
	Vertex.Position = Position;
	Vertex.Color = Color;

	// add the new entity to parent graph
	auto &Graph = ES::GetEntityMut<GraphEntity>(GraphId);
	check(!Graph.VerticesLabelToEntityId.Contains(Label));
	Graph.VerticesLabelToEntityId.Add(Label, VertexId);
	bool AlreadyInSet = false;
	Graph.Vertices.Add(VertexId, &AlreadyInSet);
	check(!AlreadyInSet);

	return VertexId;
}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Mutable::Remove"), STAT_VertexCommands_Mutable_Remove, STATGROUP_GRAPHS_PERF_COMMANDS);
void VertexCommands::Mutable::Remove(const EntityId VertexId) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Mutable_Remove);

	auto &Vertex = ES::GetEntityMut<VertexEntity>(VertexId);
	auto &Graph = ES::GetEntityMut<GraphEntity>(Vertex.GraphId);

	if (Graph.Vertices.Num() > 1) {
		// remove from the associated parent graph
		auto CheckNum = Graph.VerticesLabelToEntityId.Remove(Vertex.Label);
		check(CheckNum == 1);
		CheckNum = Graph.Vertices.Remove(VertexId);
		check(CheckNum == 1);

		// move connected edges ids because otherwise the next
		// EdgeCommands::Remove command will invalidate ConnectedEdges while we iterating it
		const auto ConnectedEdges(MoveTemp(Vertex.ConnectedEdges));
		for (const auto EdgeId : ConnectedEdges)
			EdgeCommands::Mutable::Remove(EdgeId);

		ES::RemoveEntity<VertexEntity>(VertexId);
	}
	else {
		// graph contains 1 vertex that should be removed -> remove the entire graph
		GraphCommands::Mutable::Remove(Vertex.GraphId);
	}
}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Const::Deserialize"), STAT_VertexCommands_Const_Deserialize, STATGROUP_GRAPHS_PERF_COMMANDS);
bool VertexCommands::Mutable::Deserialize(const rapidjson::Value &DomVertex, const EntityId GraphId, FString &ErrorMessage) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Const_Deserialize);

	if (!DomVertex.IsObject()) {
		ErrorMessage = "Should be an object.";
		return false;
	}

	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	uint32_t Label;
	{
		const auto &LabelMember = DomVertex.FindMember("label");
		if (LabelMember == DomVertex.MemberEnd() || !LabelMember->value.IsUint()) {
			ErrorMessage = "Object should have \"label\" unique integer number.";
			return false;
		}
		Label = LabelMember->value.GetUint();
		if (Graph.VerticesLabelToEntityId.Contains(Label)) {
			ErrorMessage = "\"label\" is not unique.";
			return false;
		}
	}

	FVector Position;
	{
		const auto &PositionMember = DomVertex.FindMember("position");
		if (PositionMember == DomVertex.MemberEnd() || !PositionMember->value.IsString()) {
			ErrorMessage = "Object should have \"position\" string.";
			return false;
		}
		if (!Position.InitFromString(FString(PositionMember->value.GetString()))) {
			ErrorMessage = "\"position\" should be in \"X=%3.3f Y=%3.3f Z=%3.3f\" format.";
			return false;
		}
	}

	FColor Color = ColorConsts::VertexDefaultColor;
	{
		const auto &ColorMember = DomVertex.FindMember("color");
		if (ColorMember != DomVertex.MemberEnd()) {
			if (!ColorMember->value.IsString() || ColorMember->value.GetStringLength() != 7) {
				ErrorMessage = "\"color\" shoud be a string in \"#RRGGBB\" format.";
				return false;
			}
			Color = FColor::FromHex(FString(ColorMember->value.GetString()));
			if (Color == FColor(ForceInitToZero)) {
				ErrorMessage = "\"color\" shoud be in #RRGGBB format.";
				return false;
			}
		}
	}

	Create(GraphId, Label, Position, Color);
	return true;
}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Const::Serialize"), STAT_VertexCommands_Const_Serialize, STATGROUP_GRAPHS_PERF_COMMANDS);
void VertexCommands::Const::Serialize(const EntityId VertexId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Const_Serialize);

	const auto &Vertex = ES::GetEntity<VertexEntity>(VertexId);
	Writer.StartObject();

	// label
	{
		Writer.Key("label");
		Writer.Uint(Vertex.Label);
	}

	// position in "X=%3.3f Y=%3.3f Z=%3.3f" format
	{
		const auto PositionStr = Vertex.Position.ToString();
		const FTCHARToUTF8 PositionStrUTF(*PositionStr);
		Writer.Key("position");
		Writer.String(PositionStrUTF.Get(), PositionStrUTF.Length());
	}

	// color in #RRGGBB format
	if (Vertex.Color != ColorConsts::VertexDefaultColor) {
		auto ColorStr = "#" + Vertex.Color.ToHex();
		ColorStr.RemoveAt(ColorStr.Len() - 2, 2, false); // removes alpha channel
		const FTCHARToUTF8 ColorStrUTF(*ColorStr);
		Writer.Key("color");
		Writer.String(ColorStrUTF.Get(), ColorStrUTF.Length());
	}

	Writer.EndObject();
}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Const::AreConnected"), STAT_VertexCommands_Const_AreConnected, STATGROUP_GRAPHS_PERF_COMMANDS);
bool VertexCommands::Const::AreConnected(const EntityId FirstVertexId, const EntityId SecondVertexId) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Const_AreConnected);
	check(FirstVertexId != SecondVertexId);

	const auto &FirstVertex = ES::GetEntity<VertexEntity>(FirstVertexId);
	const auto &SecondVertex = ES::GetEntity<VertexEntity>(SecondVertexId);

	if (FirstVertex.GraphId != SecondVertex.GraphId)
		return false;

	for (const auto EdgeId : FirstVertex.ConnectedEdges) {
		const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);
		if (Edge.ConnectedVertices[0] == SecondVertexId || Edge.ConnectedVertices[1] == SecondVertexId) {
			check(SecondVertex.ConnectedEdges.Contains(EdgeId));
			return true;
		}
	}

	return false;
}
