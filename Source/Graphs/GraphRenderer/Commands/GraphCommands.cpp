﻿#include "GraphCommands.h"
#include "VertexCommands.h"
#include "EdgeCommands.h"
#include "ThirdParty/rapidjson/error/en.h"

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Create"), STAT_GraphCommands_Create, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::Create::Create(
	EntityId *NewGraphId,
	const bool Colorful
) : GraphsRendererCommand([NewGraphId, Colorful] (AGraphsRenderer&) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Create);

	const auto GraphId = ESMut().NewEntity<GraphEntity>();
	auto &Graph = ESMut().GetEntityMut<GraphEntity>(GraphId);

	Graph.Colorful = Colorful;

	if (NewGraphId)
		*NewGraphId = GraphId;

	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Remove"), STAT_GraphCommands_Remove, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::Remove::Remove(const EntityId GraphId) : GraphsRendererCommand([GraphId] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Remove);

	const auto &Graph = ESMut().GetEntity<GraphEntity>(GraphId);
	const bool HasEdges = Graph.EdgesIds.Num() > 0;

	check(Graph.VerticesIds.Num() > 0);
	for (const auto VertexId : Graph.VerticesIds)
		ESMut().RemoveEntity<VertexEntity>(VertexId);
	for (const auto EdgeId : Graph.EdgesIds)
		ESMut().RemoveEntity<EdgeEntity>(EdgeId);
	ESMut().RemoveEntity<GraphEntity>(GraphId);

	MarkRendererComponentDirty(Renderer, {VERTEX, true, true});
	if (HasEdges)
		MarkRendererComponentDirty(Renderer, {EDGE, true, true});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::RemoveAll"), STAT_GraphCommands_RemoveAll, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::RemoveAll::RemoveAll() : GraphsRendererCommand([] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_RemoveAll);

	ESMut().Clear<GraphEntity>();
	ESMut().Clear<VertexEntity>();
	ESMut().Clear<EdgeEntity>();

	MarkRendererComponentDirty(Renderer, {VERTEX, true, true});
	MarkRendererComponentDirty(Renderer, {EDGE, true, true});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::UpdateCollisions"), STAT_GraphCommands_UpdateCollisions, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::UpdateCollisions::UpdateCollisions() : GraphsRendererCommand([] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_UpdateCollisions);
	MarkRendererComponentDirty(Renderer, {VERTEX, false, true});
	MarkRendererComponentDirty(Renderer, {EDGE, false, true});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::SetHit"), STAT_GraphCommands_SetHit, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::SetHit::SetHit(
	const EntityId GraphId,
	const bool IsHit
) : GraphsRendererCommand([GraphId, IsHit] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_SetHit);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	bool VerticesSuccess = false;
	check(Graph.VerticesIds.Num() > 0);
	ParallelFor(Graph.VerticesIds.Num(), [&Renderer, &Graph, IsHit, &VerticesSuccess] (const int32_t Idx) {
		if (Renderer.ExecuteCommand(VertexCommands::SetHit(Graph.VerticesIds[Idx], IsHit, false)))
			VerticesSuccess = true; // we don't need mutex lock here
	});
	if (VerticesSuccess)
		MarkRendererComponentDirty(Renderer, {VERTEX, true, false});

	if (Graph.EdgesIds.Num() > 0) {
		bool EdgesSuccess = false;
		ParallelFor(Graph.EdgesIds.Num(), [&Renderer, &Graph, IsHit, &EdgesSuccess] (const int32_t Idx) {
			if (Renderer.ExecuteCommand(EdgeCommands::SetHit(Graph.EdgesIds[Idx], IsHit, false)))
				EdgesSuccess = true; // we don't need mutex lock here
		});
		if (EdgesSuccess)
			MarkRendererComponentDirty(Renderer, {EDGE, true, false});
	}

	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::SetOverrideColor"), STAT_GraphCommands_SetOverrideColor, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::SetOverrideColor::SetOverrideColor(
	const EntityId GraphId,
	const FColor &OverrideColor
) : GraphsRendererCommand([GraphId, &OverrideColor] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_SetOverrideColor);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	bool VerticesSuccess = false;
	check(Graph.VerticesIds.Num() > 0);
	ParallelFor(Graph.VerticesIds.Num(), [&Renderer, &Graph, &OverrideColor, &VerticesSuccess] (const int32_t Idx) {
		if (Renderer.ExecuteCommand(VertexCommands::SetOverrideColor(Graph.VerticesIds[Idx], OverrideColor, false)))
			VerticesSuccess = true; // we don't need mutex lock here
	});
	if (VerticesSuccess)
		MarkRendererComponentDirty(Renderer, {VERTEX, true, false});

	if (Graph.EdgesIds.Num() > 0) {
		bool EdgesSuccess = false;
		ParallelFor(Graph.EdgesIds.Num(), [&Renderer, &Graph, &OverrideColor, &EdgesSuccess] (const int32_t Idx) {
			if (Renderer.ExecuteCommand(EdgeCommands::SetOverrideColor(Graph.EdgesIds[Idx], OverrideColor, false)))
				EdgesSuccess = true; // we don't need mutex lock here
		});
		if (EdgesSuccess)
			MarkRendererComponentDirty(Renderer, {EDGE, true, false});
	}

	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Move"), STAT_GraphCommands_Move, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::Move::Move(
	const EntityId GraphId,
	const FVector &Delta
) : GraphsRendererCommand([GraphId, &Delta] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Move);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	check(Graph.VerticesIds.Num() > 0);
	ParallelFor(Graph.VerticesIds.Num(), [&Graph, &Delta] (const int32_t Idx) {
		auto &Vertex = ESMut().GetEntityMut<VertexEntity>(Graph.VerticesIds[Idx]);
		Vertex.Position += Delta;
	});

	MarkRendererComponentDirty(Renderer, {VERTEX, true, false});
	if (Graph.EdgesIds.Num() > 0)
		MarkRendererComponentDirty(Renderer, {EDGE, true, false});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Rotate"), STAT_GraphCommands_Rotate, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::Rotate::Rotate(
	const EntityId GraphId,
	const FVector &Origin,
	const float Angle
) : GraphsRendererCommand([GraphId, &Origin, Angle] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Rotate);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	check(Graph.VerticesIds.Num() > 0);
	ParallelFor(Graph.VerticesIds.Num(), [&Graph, &Origin, Angle] (const int32_t Idx) {
		auto &Vertex = ESMut().GetEntityMut<VertexEntity>(Graph.VerticesIds[Idx]);
		Vertex.Position = (Vertex.Position - Origin).RotateAngleAxis(Angle, FVector::DownVector) + Origin;
	});

	MarkRendererComponentDirty(Renderer, {VERTEX, true, false});
	if (Graph.EdgesIds.Num() > 0)
		MarkRendererComponentDirty(Renderer, {EDGE, true, false});
	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Deserialize"), STAT_GraphCommands_Deserialize, STATGROUP_GRAPHS_PERF_COMMANDS);
DECLARE_CYCLE_STAT(TEXT("GraphCommands::Deserialize::ParseDom"), STAT_GraphCommands_Deserialize_ParseDom, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::Deserialize::Deserialize(
	const FString &JsonStr,
	FString &ErrorMessage
) : GraphsRendererCommand([&JsonStr, &ErrorMessage] (AGraphsRenderer &Renderer) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Deserialize);

	const FTCHARToUTF8 JsonStrUTF(*JsonStr);
	rapidjson::Document DomGraph;

	{
		SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Deserialize_ParseDom);
		if (DomGraph.Parse(JsonStrUTF.Get()).HasParseError()) {
			ErrorMessage = GetParseError_En(DomGraph.GetParseError());
			return false;
		}
	}

	if (!DomGraph.IsObject()) {
		ErrorMessage = "Graph error: Should be an object.";
		return false;
	}

	const auto &ColorfulMember = DomGraph.FindMember("colorful");
	if (ColorfulMember == DomGraph.MemberEnd() || !ColorfulMember->value.IsBool()) {
		ErrorMessage = "Graph error: Should have \"colorful\" boolean.";
		return false;
	}

	const auto &VerticesMember = DomGraph.FindMember("vertices");
	if (VerticesMember == DomGraph.MemberEnd() || !VerticesMember->value.IsArray()) {
		ErrorMessage = "Graph error: Should contain \"vertices\" array of objects.";
		return false;
	}
	const auto &DomVerticesArray = VerticesMember->value.GetArray();
	if (DomVerticesArray.Size() == 0) {
		ErrorMessage = "Graph error: \"vertices\" array should not be empty.";
		return false;
	}

	EntityId GraphId = EntityId::NONE();
	Renderer.ExecuteCommand(Create(&GraphId, ColorfulMember->value.GetBool()));
	Renderer.ExecuteCommand(VertexCommands::Reserve(GraphId, DomVerticesArray.Size()));

	VertexEntity NewVertex;
	for (const auto &DomVertex : DomVerticesArray) {
		if (!VertexCommands::ConstFuncs::Deserialize(
			GraphId,
			DomVertex,
			ErrorMessage,
			NewVertex
		)) {
			Renderer.ExecuteCommand(Remove(GraphId));
			return false;
		}

		Renderer.ExecuteCommand(VertexCommands::Create(
			GraphId, nullptr,
			NewVertex.UserId,
			NewVertex.Position,
			NewVertex.Color
		));
	}

	const auto &EdgesMember = DomGraph.FindMember("edges");
	if (EdgesMember == DomGraph.MemberEnd())
		return true;

	if (!EdgesMember->value.IsArray()) {
		ErrorMessage = "Graph error: \"edges\" should be an array of objects.";
		Renderer.ExecuteCommand(Remove(GraphId));
		return false;
	}
	const auto &DomEdgesArray = EdgesMember->value.GetArray();
	if (DomEdgesArray.Size() == 0)
		return true;

	Renderer.ExecuteCommand(EdgeCommands::Reserve(GraphId, DomEdgesArray.Size()));

	EdgeEntity NewEdge;
	for (const auto &DomEdge : DomEdgesArray) {
		if (!EdgeCommands::ConstFuncs::Deserialize(
			GraphId,
			DomEdge,
			ErrorMessage,
			NewEdge
		)) {
			Renderer.ExecuteCommand(Remove(GraphId));
			return false;
		}

		Renderer.ExecuteCommand(EdgeCommands::Create(
			GraphId, nullptr,
			NewEdge.VerticesIds[0], NewEdge.VerticesIds[1]
		));
	}

	return true;
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::ConstFuncs::Serialize"), STAT_GraphCommands_ConstFuncs_Serialize, STATGROUP_GRAPHS_PERF_COMMANDS);
void GraphCommands::ConstFuncs::Serialize(
	const EntityId GraphId,
	rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_ConstFuncs_Serialize);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	Writer.StartObject();

	Writer.Key("colorful");
	Writer.Bool(Graph.Colorful);

	check(Graph.VerticesIds.Num() > 0);
	Writer.Key("vertices");
	Writer.StartArray();
	for (const auto VertexId : Graph.VerticesIds)
		VertexCommands::ConstFuncs::Serialize(VertexId, Writer);
	Writer.EndArray();

	if (Graph.EdgesIds.Num() > 0) {
		Writer.Key("edges");
		Writer.StartArray();
		for (const auto EdgeId : Graph.EdgesIds)
			EdgeCommands::ConstFuncs::Serialize(EdgeId, Writer);
		Writer.EndArray();
	}

	Writer.EndObject();
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::ConstFuncs::ComputeCenterPosition"), STAT_GraphCommands_ConstFuncs_ComputeCenterPosition, STATGROUP_GRAPHS_PERF_COMMANDS);
FVector GraphCommands::ConstFuncs::ComputeCenterPosition(const EntityId GraphId) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_ConstFuncs_ComputeCenterPosition);

	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);
	check(Graph.VerticesIds.Num() > 0);

	FVector Center = FVector::ZeroVector;
	for (const auto &VertexId : Graph.VerticesIds)
		Center += ES::GetEntity<VertexEntity>(VertexId).Position;
	Center /= Graph.VerticesIds.Num();

	return Center;
}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::ConstFuncs::IsSetContainsGraphChildrenEntities"), STAT_GraphCommands_ConstFuncs_IsSetContainsGraphChildrenEntities, STATGROUP_GRAPHS_PERF_COMMANDS);
bool GraphCommands::ConstFuncs::IsSetContainsGraphChildrenEntities(
	const EntityId GraphId,
	const TSet<EntityId> &InSet
) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_ConstFuncs_IsSetContainsGraphChildrenEntities);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	if (InSet.Num() < Graph.VerticesIds.Num() + Graph.EdgesIds.Num())
		return false;

	for (const auto &EdgeId : Graph.EdgesIds)
		if (!InSet.Contains(EdgeId))
			return false;

	for (const auto &VertexId : Graph.VerticesIds)
		if (!InSet.Contains(VertexId))
			return false;

	return true;
}
