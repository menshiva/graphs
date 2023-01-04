#include "GraphCommands.h"
#include "EdgeCommands.h"
#include "VertexCommands.h"
#include "ThirdParty/rapidjson/error/en.h"

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::Create"), STAT_GraphCommands_Mutable_Create, STATGROUP_GRAPHS_PERF_COMMANDS);
EntityId GraphCommands::Mutable::Create(const bool Colorful) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_Create);

	// create a new graph entity
	const auto GraphId = ES::NewEntity<GraphEntity>();

	// fill the new entity with given properties
	ES::GetEntityMut<GraphEntity>(GraphId).Colorful = Colorful;

	return GraphId;
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::Remove"), STAT_GraphCommands_Mutable_Remove, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Mutable::Remove(const EntityId GraphId) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_Remove);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);
	for (const auto VertexId : Graph.Vertices)
		ES::RemoveEntity<VertexEntity>(VertexId);
	for (const auto EdgeId : Graph.Edges)
		ES::RemoveEntity<EdgeEntity>(EdgeId);
	ES::RemoveEntity<GraphEntity>(GraphId);
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::SetHit"), STAT_GraphCommands_Mutable_SetHit, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Mutable::SetHit(const EntityId GraphId, const bool IsHit) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_SetHit);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	check(Graph.Vertices.Num() > 0);
	for (const auto VertexId : Graph.Vertices)
		VertexCommands::Mutable::SetHit(VertexId, IsHit);

	for (const auto EdgeId : Graph.Edges)
		EdgeCommands::Mutable::SetHit(EdgeId, IsHit);
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::SetColor"), STAT_GraphCommands_Mutable_SetColor, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Mutable::SetColor(const EntityId GraphId, const FColor &Color) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_SetColor);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	check(Graph.Vertices.Num() > 0);
	for (const auto VertexId : Graph.Vertices)
		VertexCommands::Mutable::SetColor(VertexId, Color);
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::SetColor_Arr"), STAT_GraphCommands_Mutable_SetColor_Arr, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Mutable::SetColor(const EntityId GraphId, const TArray<FColor> &Colors) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_SetColor_Arr);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	check(Graph.Vertices.Num() > 0);
	check(Graph.Vertices.Num() == Colors.Num());
	auto ColorIter = Colors.CreateConstIterator();
	for (const auto VertexId : Graph.Vertices)
		VertexCommands::Mutable::SetColor(VertexId, *ColorIter++);
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::RandomizeVerticesColors"), STAT_GraphCommands_Mutable_RandomizeVerticesColors, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Mutable::RandomizeVerticesColors(const EntityId GraphId) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_RandomizeVerticesColors);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	check(Graph.Vertices.Num() > 0);
	for (const auto VertexId : Graph.Vertices)
		VertexCommands::Mutable::SetColor(VertexId, FLinearColor::MakeRandomColor().ToFColor(false));
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::SetOverrideColor"), STAT_GraphCommands_Mutable_SetOverrideColor, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Mutable::SetOverrideColor(const EntityId GraphId, const FColor &OverrideColor) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_SetOverrideColor);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	check(Graph.Vertices.Num() > 0);
	for (const auto VertexId : Graph.Vertices)
		VertexCommands::Mutable::SetOverrideColor(VertexId, OverrideColor);

	for (const auto EdgeId : Graph.Edges)
		EdgeCommands::Mutable::SetOverrideColor(EdgeId, OverrideColor);
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::Move"), STAT_GraphCommands_Mutable_Move, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Mutable::Move(const EntityId GraphId, const FVector &Delta) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_Move);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	check(Graph.Vertices.Num() > 0);
	for (const auto VertexId : Graph.Vertices)
		VertexCommands::Mutable::Move(VertexId, Delta);
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::Rotate"), STAT_GraphCommands_Mutable_Rotate, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Mutable::Rotate(
	const EntityId GraphId,
	const FVector &Origin, const FVector &Axis,
	const float Angle
) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_Rotate);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	check(Graph.Vertices.Num() > 0);
	for (const auto VertexId : Graph.Vertices) {
		auto &Vertex = ES::GetEntityMut<VertexEntity>(VertexId);
		Vertex.Position = (Vertex.Position - Origin).RotateAngleAxis(Angle, Axis) + Origin;
	}
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::Deserialize"), STAT_GraphCommands_Mutable_Deserialize, STATGROUP_GRAPHS_PERF_COMMANDS);
DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::Deserialize_ParseDom"), STAT_GraphCommands_Mutable_Deserialize_ParseDom, STATGROUP_GRAPHS_PERF_COMMANDS);
EntityId GraphCommands::Mutable::Deserialize(const FString &JsonStr, FString &ErrorMessage) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_Deserialize);

	const FTCHARToUTF8 JsonStrUTF(*JsonStr);
	rapidjson::Document DomGraph;

	{
		SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_Deserialize_ParseDom);
		if (DomGraph.Parse(JsonStrUTF.Get()).HasParseError()) {
			ErrorMessage = GetParseError_En(DomGraph.GetParseError());
			return EntityId::NONE();
		}
	}

	if (!DomGraph.IsObject()) {
		ErrorMessage = "Graph error: Should be an object.";
		return EntityId::NONE();
	}

	bool Colorful = false;
	{
		const auto &ColorfulMember = DomGraph.FindMember("colorful");
		if (ColorfulMember != DomGraph.MemberEnd()) {
			if (!ColorfulMember->value.IsBool()) {
				ErrorMessage = "Graph error: \"colorful\" should be a boolean.";
				return EntityId::NONE();
			}
			Colorful = ColorfulMember->value.GetBool();
		}
	}

	const auto GraphId = Create(Colorful);
	auto &Graph = ES::GetEntityMut<GraphEntity>(GraphId);

	// vertices
	{
		const auto &VerticesMember = DomGraph.FindMember("vertices");
		if (VerticesMember == DomGraph.MemberEnd() || !VerticesMember->value.IsArray()) {
			ErrorMessage = "Graph error: Should have \"vertices\" array of objects.";
			Remove(GraphId);
			return EntityId::NONE();
		}
		const auto &DomVerticesArray = VerticesMember->value.GetArray();
		if (DomVerticesArray.Size() == 0) {
			ErrorMessage = "Graph error: \"vertices\" array should not be empty.";
			Remove(GraphId);
			return EntityId::NONE();
		}

		ES::Reserve<VertexEntity>(DomVerticesArray.Size());
		Graph.VerticesLabelToEntityId.Reserve(DomVerticesArray.Size());
		Graph.Vertices.Reserve(DomVerticesArray.Size());

		size_t VertexI = 0;
		for (const auto &DomVertex : DomVerticesArray) {
			if (!VertexCommands::Mutable::Deserialize(DomVertex, GraphId, ErrorMessage)) {
				ErrorMessage = "Vertex #" + FString::FromInt(VertexI) + " error: " + ErrorMessage;
				Remove(GraphId);
				return EntityId::NONE();
			}
			++VertexI;
		}
	}

	// edges
	{
		const auto &EdgesMember = DomGraph.FindMember("edges");
		if (EdgesMember == DomGraph.MemberEnd())
			return GraphId;

		if (!EdgesMember->value.IsArray()) {
			ErrorMessage = "Graph error: \"edges\" should be an array of objects.";
			Remove(GraphId);
			return EntityId::NONE();
		}
		const auto &DomEdgesArray = EdgesMember->value.GetArray();
		if (DomEdgesArray.Size() == 0)
			return GraphId;

		ES::Reserve<EdgeEntity>(DomEdgesArray.Size());
		Graph.EdgesHashes.Reserve(DomEdgesArray.Size());
		Graph.Edges.Reserve(DomEdgesArray.Size());

		size_t EdgeI = 0;
		for (const auto &DomEdge : DomEdgesArray) {
			if (!EdgeCommands::Mutable::Deserialize(DomEdge, GraphId, ErrorMessage)) {
				ErrorMessage = "Edge #" + FString::FromInt(EdgeI) + " error: " + ErrorMessage;
				Remove(GraphId);
				return EntityId::NONE();
			}
			++EdgeI;
		}
	}

	return GraphId;
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Const::Serialize"), STAT_GraphCommands_Const_Serialize, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Const::Serialize(const EntityId GraphId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Const_Serialize);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	Writer.StartObject();

	// colorful
	Writer.Key("colorful");
	Writer.Bool(Graph.Colorful);

	// vertices
	{
		check(Graph.Vertices.Num() > 0);

		// copy vertices ids and sort them by the vertex' label so that vertices are arranged in ascending order in the file
		auto VerticesSortedByLabel = Graph.Vertices.Array();
		VerticesSortedByLabel.Sort([] (const EntityId F, const EntityId S) {
			return ES::GetEntity<VertexEntity>(F).Label < ES::GetEntity<VertexEntity>(S).Label;
		});

		Writer.Key("vertices");
		Writer.StartArray();
		for (const auto VertexId : VerticesSortedByLabel)
			VertexCommands::Const::Serialize(VertexId, Writer);
		Writer.EndArray();
	}

	// edges
	if (Graph.Edges.Num() > 0) {
		Writer.Key("edges");
		Writer.StartArray();
		for (const auto EdgeId : Graph.Edges)
			EdgeCommands::Const::Serialize(EdgeId, Writer);
		Writer.EndArray();
	}

	Writer.EndObject();
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Const::ComputeCenterPosition"), STAT_GraphCommands_Const_ComputeCenterPosition, STATGROUP_GRAPHS_PERF_COMMANDS);
FVector GraphCommands::Const::ComputeCenterPosition(const EntityId GraphId) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Const_ComputeCenterPosition);

	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);
	check(Graph.Vertices.Num() > 0);

	FVector Center = FVector::ZeroVector;
	for (const auto VertexId : Graph.Vertices)
		Center += ES::GetEntity<VertexEntity>(VertexId).Position;
	Center /= Graph.Vertices.Num();

	return Center;
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Const::GenerateUniqueVertexUserId"), STAT_GraphCommands_Const_GenerateUniqueVertexLabel, STATGROUP_GRAPHS_PERF_COMMANDS);
uint32_t GraphCommands::Const::GenerateUniqueVertexLabel(const EntityId GraphId) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Const_GenerateUniqueVertexLabel);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	// we simply try to find the highest vertex label in the graph
	uint32_t MaxLabel = -1;
	if (Graph.VerticesLabelToEntityId.Num() > 0) {
		auto Iter = Graph.VerticesLabelToEntityId.CreateConstIterator();
		MaxLabel = Iter.Key();
		for (++Iter; Iter; ++Iter)
			if (Iter.Key() > MaxLabel)
				MaxLabel = Iter.Key();
	}

	// and then return it + 1
	return MaxLabel + 1;
}
