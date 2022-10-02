#include "GraphCommands.h"
#include "EdgeCommands.h"
#include "VertexCommands.h"
#include "ThirdParty/rapidjson/error/en.h"

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::Create"), STAT_GraphCommands_Mutable_Create, STATGROUP_GRAPHS_PERF_COMMANDS);
EntityId GraphCommands::Mutable::Create(const bool Colorful) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_Create);

	// create new graph entity
	const auto GraphId = ES::NewEntity<GraphEntity>();

	// fill new entity with given properties
	ES::GetEntityMut<GraphEntity>(GraphId).Colorful = Colorful;

	return GraphId;
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::Remove"), STAT_GraphCommands_Mutable_Remove, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Mutable::Remove(const EntityId GraphId) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_Remove);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	check(Graph.Vertices.Num() > 0);
	for (const auto VertexId : Graph.Vertices)
		ES::RemoveEntity<VertexEntity>(VertexId);
	for (const auto EdgeId : Graph.Edges)
		ES::RemoveEntity<EdgeEntity>(EdgeId);
	ES::RemoveEntity<GraphEntity>(GraphId);
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::SetHit"), STAT_GraphCommands_Mutable_SetHit, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Mutable::SetHit(const EntityId GraphId, const bool IsHit) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_SetHit);
	auto &Graph = ES::GetEntityMut<GraphEntity>(GraphId);

	check(Graph.Vertices.Num() > 0);
	ParallelFor(Graph.Vertices.Num(), [&Graph, IsHit] (const int32_t Idx) {
		VertexCommands::Mutable::SetHit(Graph.Vertices[FSetElementId::FromInteger(Idx)], IsHit);
	});

	if (Graph.Edges.Num() > 0) {
		ParallelFor(Graph.Edges.Num(), [&Graph, IsHit] (const int32_t Idx) {
			EdgeCommands::Mutable::SetHit(Graph.Edges[FSetElementId::FromInteger(Idx)], IsHit);
		});
	}
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::SetOverrideColor"), STAT_GraphCommands_Mutable_SetOverrideColor, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Mutable::SetOverrideColor(const EntityId GraphId, const FColor &OverrideColor) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_SetOverrideColor);
	auto &Graph = ES::GetEntityMut<GraphEntity>(GraphId);

	check(Graph.Vertices.Num() > 0);
	ParallelFor(Graph.Vertices.Num(), [&Graph, &OverrideColor] (const int32_t Idx) {
		VertexCommands::Mutable::SetOverrideColor(Graph.Vertices[FSetElementId::FromInteger(Idx)], OverrideColor);
	});

	if (Graph.Edges.Num() > 0) {
		ParallelFor(Graph.Edges.Num(), [&Graph, &OverrideColor] (const int32_t Idx) {
			EdgeCommands::Mutable::SetOverrideColor(Graph.Edges[FSetElementId::FromInteger(Idx)], OverrideColor);
		});
	}
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::Move"), STAT_GraphCommands_Mutable_Move, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Mutable::Move(const EntityId GraphId, const FVector &Delta) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_Move);
	auto &Graph = ES::GetEntityMut<GraphEntity>(GraphId);

	check(Graph.Vertices.Num() > 0);
	ParallelFor(Graph.Vertices.Num(), [&Graph, &Delta] (const int32_t Idx) {
		VertexCommands::Mutable::Move(Graph.Vertices[FSetElementId::FromInteger(Idx)], Delta);
	});
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::Rotate"), STAT_GraphCommands_Mutable_Rotate, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Mutable::Rotate(const EntityId GraphId, const FVector &Origin, const float Angle) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Mutable_Rotate);
	auto &Graph = ES::GetEntityMut<GraphEntity>(GraphId);

	check(Graph.Vertices.Num() > 0);
	ParallelFor(Graph.Vertices.Num(), [&Graph, &Origin, Angle] (const int32_t Idx) {
		auto &Vertex = ES::GetEntityMut<VertexEntity>(Graph.Vertices[FSetElementId::FromInteger(Idx)]);
		Vertex.Position = (Vertex.Position - Origin).RotateAngleAxis(Angle, FVector::DownVector) + Origin;
	});
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::CompactSets"), STAT_GraphCommands_Mutable_CompactSets, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Mutable::CompactSets(const EntityId GraphId) {
	auto &Graph = ES::GetEntityMut<GraphEntity>(GraphId);
	// make sets be a contiguous range
	Graph.Vertices.Compact();
	Graph.Edges.Compact();
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Mutable::Deserialize"), STAT_GraphCommands_Mutable_Deserialize,
                   STATGROUP_GRAPHS_PERF_COMMANDS);
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

	bool Colorful;
	{
		const auto &ColorfulMember = DomGraph.FindMember("colorful");
		if (ColorfulMember == DomGraph.MemberEnd() || !ColorfulMember->value.IsBool()) {
			ErrorMessage = "Graph error: Should have \"colorful\" boolean.";
			return EntityId::NONE();
		}
		Colorful = ColorfulMember->value.GetBool();
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
		Graph.VerticesCustomIdToEntityId.Reserve(DomVerticesArray.Size());
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

		// copy vertices ids and sort them by CustomId
		auto VerticesSortedByCustomId = Graph.Vertices.Array();
		VerticesSortedByCustomId.Sort([] (const EntityId F, const EntityId S) {
			return ES::GetEntity<VertexEntity>(F).CustomId < ES::GetEntity<VertexEntity>(S).CustomId;
		});

		Writer.Key("vertices");
		Writer.StartArray();
		for (const auto VertexId : VerticesSortedByCustomId)
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

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Const::GenerateUniqueVertexUserId"), STAT_GraphCommands_Const_GenerateUniqueVertexUserId, STATGROUP_GRAPHS_PERF_COMMANDS);
uint32_t GraphCommands::Const::GenerateUniqueVertexUserId(const EntityId GraphId) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Const_GenerateUniqueVertexUserId);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	uint32_t MaxUserId = -1;
	if (Graph.VerticesCustomIdToEntityId.Num() > 0) {
		auto Iter = Graph.VerticesCustomIdToEntityId.CreateConstIterator();
		MaxUserId = Iter.Key();
		for (++Iter; Iter; ++Iter)
			if (Iter.Key() > MaxUserId)
				MaxUserId = Iter.Key();
	}

	return MaxUserId;
}
