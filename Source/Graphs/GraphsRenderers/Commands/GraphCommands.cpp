#include "GraphCommands.h"
#include "VertexCommands.h"
#include "EdgeCommands.h"
#include "ThirdParty/rapidjson/error/en.h"

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Create"), STAT_GraphCommands_Create, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::Create::Create(EntityId *NewGraphId, const bool Colorful) : GraphsRenderersCommand([=] {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Create);

	const auto GraphId = GetESMut().NewEntity<GraphEntity>();
	auto &Graph = GetESMut().GetEntityMut<GraphEntity>(GraphId);

	Graph.Colorful = Colorful;

	if (NewGraphId)
		*NewGraphId = GraphId;
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::FillFromImportData"), STAT_GraphCommands_FillFromImportData, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::FillFromImportData::FillFromImportData(
	const EntityId GraphId,
	const GraphImportData &ImportData
) : GraphsRenderersCommand([=, &ImportData] {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_FillFromImportData);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	check(Graph.VerticesCustomIdToEntityId.Num() == 0);
	check(Graph.EdgesHashes.Num() == 0);
	check(Graph.Colorful == ImportData.Colorful);
	check(Graph.Vertices.Num() == 0);
	check(Graph.Edges.Num() == 0);

	ExecuteSubCommand(VertexCommands::Reserve(GraphId, ImportData.Vertices.Num()));
	ExecuteSubCommand(EdgeCommands::Reserve(GraphId, ImportData.Edges.Num()));

	for (const auto &VertexImportData : ImportData.Vertices) {
		ExecuteSubCommand(VertexCommands::Create(
			GraphId, nullptr,
			VertexImportData.CustomId,
			VertexImportData.Position,
			VertexImportData.Color
		));
	}

	for (const auto &EdgeImportData : ImportData.Edges) {
		ExecuteSubCommand(EdgeCommands::Create(
			GraphId, nullptr,
			Graph.VerticesCustomIdToEntityId.FindChecked(EdgeImportData.ConnectedVertexCustomIds[0]),
			Graph.VerticesCustomIdToEntityId.FindChecked(EdgeImportData.ConnectedVertexCustomIds[1])
		));
	}

	const auto GraphRenderer = GetGraphRenderer(GraphId);
	GraphRenderer->MarkDirty({VERTEX, true, true});
	if (Graph.Edges.Num() > 0)
		GraphRenderer->MarkDirty({EDGE, true, true});
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Remove"), STAT_GraphCommands_Remove, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::Remove::Remove(const EntityId GraphId) : GraphsRenderersCommand([=] {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Remove);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	check(Graph.Vertices.Num() > 0);
	for (const auto VertexId : Graph.Vertices)
		GetESMut().RemoveEntity<VertexEntity>(VertexId);
	for (const auto EdgeId : Graph.Edges)
		GetESMut().RemoveEntity<EdgeEntity>(EdgeId);
	GetESMut().RemoveEntity<GraphEntity>(GraphId);
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::RemoveAll"), STAT_GraphCommands_RemoveAll, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::RemoveAll::RemoveAll() : GraphsRenderersCommand([] {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Remove);

	GetESMut().Clear<VertexEntity>();
	GetESMut().Clear<EdgeEntity>();
	GetESMut().Clear<GraphEntity>();
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::UpdateCollisions"), STAT_GraphCommands_UpdateCollisions, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::UpdateCollisions::UpdateCollisions(const EntityId GraphId) : GraphsRenderersCommand([=] {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_UpdateCollisions);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	const auto GraphRenderer = GetGraphRenderer(GraphId);
	GraphRenderer->MarkDirty({VERTEX, false, true});
	if (Graph.Edges.Num() > 0)
		GraphRenderer->MarkDirty({EDGE, false, true});
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::SetHit"), STAT_GraphCommands_SetHit, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::SetHit::SetHit(const EntityId GraphId, const bool IsHit) : GraphsRenderersCommand([=] {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_SetHit);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);
	const auto GraphRenderer = GetGraphRenderer(GraphId);

	check(Graph.Vertices.Num() > 0);
	ParallelFor(Graph.Vertices.Num(), [&Graph, IsHit] (const int32_t Idx) {
		GetESMut().GetEntityMut<VertexEntity>(Graph.Vertices[Idx]).IsHit = IsHit;
	});
	GraphRenderer->MarkDirty({VERTEX, true, false});

	if (Graph.Edges.Num() > 0) {
		ParallelFor(Graph.Edges.Num(), [&Graph, IsHit] (const int32_t Idx) {
			GetESMut().GetEntityMut<EdgeEntity>(Graph.Edges[Idx]).IsHit = IsHit;
		});
		GraphRenderer->MarkDirty({EDGE, true, false});
	}
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::SetOverrideColor"), STAT_GraphCommands_SetOverrideColor, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::SetOverrideColor::SetOverrideColor(
	const EntityId GraphId,
	const FColor &OverrideColor
) : GraphsRenderersCommand([=, &OverrideColor] {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_SetOverrideColor);

	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);
	const auto GraphRenderer = GetGraphRenderer(GraphId);

	check(Graph.Vertices.Num() > 0);
	ParallelFor(Graph.Vertices.Num(), [&Graph, &OverrideColor] (const int32_t Idx) {
		GetESMut().GetEntityMut<VertexEntity>(Graph.Vertices[Idx]).OverrideColor = OverrideColor;
	});
	GraphRenderer->MarkDirty({VERTEX, true, false});

	if (Graph.Edges.Num() > 0) {
		ParallelFor(Graph.Edges.Num(), [&Graph, &OverrideColor] (const int32_t Idx) {
			GetESMut().GetEntityMut<EdgeEntity>(Graph.Edges[Idx]).OverrideColor = OverrideColor;
		});
		GraphRenderer->MarkDirty({EDGE, true, false});
	}
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Move"), STAT_GraphCommands_Move, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::Move::Move(const EntityId GraphId, const FVector &Delta) : GraphsRenderersCommand([=, &Delta] {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Move);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	check(Graph.Vertices.Num() > 0);
	ParallelFor(Graph.Vertices.Num(), [&Graph, &Delta] (const int32_t Idx) {
		auto &Vertex = GetESMut().GetEntityMut<VertexEntity>(Graph.Vertices[Idx]);
		Vertex.Position += Delta;
	});

	const auto GraphRenderer = GetGraphRenderer(GraphId);
	GraphRenderer->MarkDirty({VERTEX, true, false});
	if (Graph.Edges.Num() > 0)
		GraphRenderer->MarkDirty({EDGE, true, false});
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Rotate"), STAT_GraphCommands_Rotate, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::Rotate::Rotate(
	const EntityId GraphId,
	const FVector &Origin,
	const float Angle
) : GraphsRenderersCommand([=, &Origin] {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Rotate);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	check(Graph.Vertices.Num() > 0);
	ParallelFor(Graph.Vertices.Num(), [&Graph, &Origin, Angle] (const int32_t Idx) {
		auto &Vertex = GetESMut().GetEntityMut<VertexEntity>(Graph.Vertices[Idx]);
		Vertex.Position = (Vertex.Position - Origin).RotateAngleAxis(Angle, FVector::DownVector) + Origin;
	});

	const auto GraphRenderer = GetGraphRenderer(GraphId);
	GraphRenderer->MarkDirty({VERTEX, true, false});
	if (Graph.Edges.Num() > 0)
		GraphRenderer->MarkDirty({EDGE, true, false});
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Consts::Serialize"), STAT_GraphCommands_Consts_Serialize, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::Consts::Serialize(const EntityId GraphId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Consts_Serialize);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	Writer.StartObject();

	// colorful
	Writer.Key("colorful");
	Writer.Bool(Graph.Colorful);

	// vertices
	{
		check(Graph.Vertices.Num() > 0);

		// copy vertices ids and sort them by vertex' CustomId
		auto VerticesSortedByCustomId = Graph.Vertices;
		VerticesSortedByCustomId.Sort([] (const EntityId F, const EntityId S) {
			return ES::GetEntity<VertexEntity>(F).CustomId < ES::GetEntity<VertexEntity>(S).CustomId;
		});

		Writer.Key("vertices");
		Writer.StartArray();
		for (const auto VertexId : VerticesSortedByCustomId)
			VertexCommands::Consts::Serialize(VertexId, Writer);
		Writer.EndArray();
	}

	// edges
	if (Graph.Edges.Num() > 0) {
		Writer.Key("edges");
		Writer.StartArray();
		for (const auto EdgeId : Graph.Edges)
			EdgeCommands::Consts::Serialize(EdgeId, Writer);
		Writer.EndArray();
	}

	Writer.EndObject();
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Consts::Deserialize"), STAT_GraphCommands_Consts_Deserialize, STATGROUP_GRAPHS_PERF_COMMANDS);
DECLARE_CYCLE_STAT(TEXT("GraphCommands::Consts::Deserialize_ParseDom"), STAT_GraphCommands_Consts_Deserialize_ParseDom, STATGROUP_GRAPHS_PERF_COMMANDS);
bool GraphCommands::Consts::Deserialize(const FString &JsonStr, GraphImportData &ImportData, FString &ErrorMessage) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Consts_Deserialize);

	const FTCHARToUTF8 JsonStrUTF(*JsonStr);
	rapidjson::Document DomGraph;

	{
		SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Consts_Deserialize_ParseDom);
		if (DomGraph.Parse(JsonStrUTF.Get()).HasParseError()) {
			ErrorMessage = GetParseError_En(DomGraph.GetParseError());
			return false;
		}
	}

	if (!DomGraph.IsObject()) {
		ErrorMessage = "Graph error: Should be an object.";
		return false;
	}

	// colorful
	{
		const auto &ColorfulMember = DomGraph.FindMember("colorful");
		if (ColorfulMember == DomGraph.MemberEnd() || !ColorfulMember->value.IsBool()) {
			ErrorMessage = "Graph error: Should have \"colorful\" boolean.";
			return false;
		}
		ImportData.Colorful = ColorfulMember->value.GetBool();
	}

	TSet<uint32_t> VerticesCustomIds;

	// vertices
	{
		const auto &VerticesMember = DomGraph.FindMember("vertices");
		if (VerticesMember == DomGraph.MemberEnd() || !VerticesMember->value.IsArray()) {
			ErrorMessage = "Graph error: Should have \"vertices\" array of objects.";
			return false;
		}
		const auto &DomVerticesArray = VerticesMember->value.GetArray();
		if (DomVerticesArray.Size() == 0) {
			ErrorMessage = "Graph error: \"vertices\" array should not be empty.";
			return false;
		}

		check(ImportData.Vertices.Num() == 0);
		VerticesCustomIds.Reserve(DomVerticesArray.Size());
		ImportData.Vertices.Reserve(DomVerticesArray.Size());

		size_t VertexI = 0;
		for (const auto &DomVertex : DomVerticesArray) {
			if (!VertexCommands::Consts::Deserialize(DomVertex, ImportData, VerticesCustomIds, ErrorMessage)) {
				ErrorMessage = "Vertex #" + FString::FromInt(VertexI) + " error: " + ErrorMessage;
				return false;
			}
			++VertexI;
		}
	}

	// edges
	{
		const auto &EdgesMember = DomGraph.FindMember("edges");
		if (EdgesMember == DomGraph.MemberEnd())
			return true;

		if (!EdgesMember->value.IsArray()) {
			ErrorMessage = "Graph error: \"edges\" should be an array of objects.";
			return false;
		}
		const auto &DomEdgesArray = EdgesMember->value.GetArray();
		if (DomEdgesArray.Size() == 0)
			return true;

		TSet<uint32_t> ImportEdgesHashes;
		ImportEdgesHashes.Reserve(DomEdgesArray.Size());
		check(ImportData.Edges.Num() == 0);
		ImportData.Edges.Reserve(DomEdgesArray.Size());

		size_t EdgeI = 0;
		for (const auto &DomEdge : DomEdgesArray) {
			if (!EdgeCommands::Consts::Deserialize(
				DomEdge,
				ImportData,
				VerticesCustomIds,
				ImportEdgesHashes,
				ErrorMessage
			)) {
				ErrorMessage = "Edge #" + FString::FromInt(EdgeI) + " error: " + ErrorMessage;
				return false;
			}
			++EdgeI;
		}
	}

	return true;
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Consts::ComputeCenterPosition"),
                   STAT_GraphCommands_Consts_ComputeCenterPosition, STATGROUP_GRAPHS_PERF_COMMANDS);
FVector GraphCommands::Consts::ComputeCenterPosition(const EntityId GraphId) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Consts_ComputeCenterPosition);

	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);
	check(Graph.Vertices.Num() > 0);

	FVector Center = FVector::ZeroVector;
	for (const auto VertexId : Graph.Vertices)
		Center += ES::GetEntity<VertexEntity>(VertexId).Position;
	Center /= Graph.Vertices.Num();

	return Center;
}
