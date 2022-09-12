#include "GraphCommands.h"
#include "VertexCommands.h"
#include "EdgeCommands.h"
#include "ThirdParty/rapidjson/error/en.h"

GraphCommands::Create::Create(
	EntityId *NewGraphId,
	const bool UseDefaultColors
) : Command([&, NewGraphId, UseDefaultColors] (EntityStorage &Storage) -> bool {
	const auto GraphId = Storage.NewEntity<GraphEntity>();

	auto &Graph = Storage.GetEntityMut<GraphEntity>(GraphId);
	Graph.UseDefaultColors = UseDefaultColors;

	if (NewGraphId)
		*NewGraphId = GraphId;

	return true;
}) {}

GraphCommands::Remove::Remove(const EntityId &GraphId) : Command([&] (EntityStorage &Storage) -> bool {
	const auto &Graph = Storage.GetEntity<GraphEntity>(GraphId);

	for (const auto &VertexId : Graph.VerticesIds)
		Storage.RemoveEntity<VertexEntity>(VertexId);
	for (const auto &EdgeId : Graph.EdgesIds)
		Storage.RemoveEntity<EdgeEntity>(EdgeId);
	Storage.RemoveEntity<GraphEntity>(GraphId);

	return true;
}) {}

GraphCommands::SetHit::SetHit(
	const EntityId &GraphId,
	const bool IsHit
) : Command([&, IsHit] (EntityStorage &Storage) -> bool {
	const auto &Graph = Storage.GetEntity<GraphEntity>(GraphId);
	bool ChangesProvided = false;

	for (const auto &VertexId : Graph.VerticesIds) {
		const bool IsSuccess = ExecuteSubCommand(VertexCommands::SetHit(VertexId, IsHit), Storage);
		if (!ChangesProvided)
			ChangesProvided = IsSuccess;
	}

	for (const auto &EdgeId : Graph.EdgesIds) {
		const bool IsSuccess = ExecuteSubCommand(EdgeCommands::SetHit(EdgeId, IsHit), Storage);
		if (!ChangesProvided)
			ChangesProvided = IsSuccess;
	}

	return ChangesProvided;
}) {}

GraphCommands::SetOverrideColor::SetOverrideColor(
	const EntityId &GraphId,
	const FLinearColor &OverrideColor
) : Command([&] (EntityStorage &Storage) -> bool {
	const auto &Graph = Storage.GetEntity<GraphEntity>(GraphId);
	bool ChangesProvided = false;

	for (const auto &VertexId : Graph.VerticesIds) {
		const bool IsSuccess = ExecuteSubCommand(VertexCommands::SetOverrideColor(VertexId, OverrideColor), Storage);
		if (!ChangesProvided)
			ChangesProvided = IsSuccess;
	}

	for (const auto &EdgeId : Graph.EdgesIds) {
		const bool IsSuccess = ExecuteSubCommand(EdgeCommands::SetOverrideColor(EdgeId, OverrideColor), Storage);
		if (!ChangesProvided)
			ChangesProvided = IsSuccess;
	}

	return ChangesProvided;
}) {}

GraphCommands::Move::Move(
	const EntityId &GraphId,
	const FVector &Delta
) : Command([&] (EntityStorage &Storage) -> bool {
	const auto &Graph = Storage.GetEntity<GraphEntity>(GraphId);

	for (const auto &VertexId : Graph.VerticesIds)
		ExecuteSubCommand(VertexCommands::Move(VertexId, Delta), Storage);

	return true;
}) {}

GraphCommands::Rotate::Rotate(
	const EntityId &GraphId,
	const FVector &Origin,
	const float Angle
) : Command([&, Angle] (EntityStorage &Storage) -> bool {
	const auto &Graph = Storage.GetEntity<GraphEntity>(GraphId);

	for (const auto &VertexId : Graph.VerticesIds) {
		const auto &VertexPos = Storage.GetEntity<VertexEntity>(VertexId).Position;
		const auto RotatedPos = (VertexPos - Origin).RotateAngleAxis(Angle, FVector::DownVector) + Origin;
		ExecuteSubCommand(VertexCommands::Move(VertexId, RotatedPos - VertexPos), Storage);
	}

	return true;
}) {}

GraphCommands::Deserialize::Deserialize(
	const FString &JsonStr,
	FString &ErrorMessage
) : Command([&] (EntityStorage &Storage) -> bool {
	const FTCHARToUTF8 JsonStrUTF(*JsonStr);
	rapidjson::Document DomGraph;

	if (DomGraph.Parse(JsonStrUTF.Get()).HasParseError()) {
		ErrorMessage = GetParseError_En(DomGraph.GetParseError());
		return false;
	}

	if (!DomGraph.IsObject()) {
		ErrorMessage = "Graph should be an object.";
		return false;
	}

	const auto &ColorfulMember = DomGraph.FindMember("colorful");
	if (ColorfulMember == DomGraph.MemberEnd() || !ColorfulMember->value.IsBool()) {
		ErrorMessage = "Graph should have \"colorful\" boolean.";
		return false;
	}

	const auto &VerticesMember = DomGraph.FindMember("vertices");
	if (VerticesMember == DomGraph.MemberEnd() || !VerticesMember->value.IsArray()) {
		ErrorMessage = "Graph should contain \"vertices\" array of objects.";
		return false;
	}
	const auto &DomVerticesArray = VerticesMember->value.GetArray();
	if (DomVerticesArray.Size() == 0) {
		ErrorMessage = "\"vertices\" array should not be empty.";
		return false;
	}

	EntityId GraphId;
	ExecuteSubCommand(Create(&GraphId, ColorfulMember->value.GetBool()), Storage);
	ExecuteSubCommand(VertexCommands::Reserve(GraphId, DomVerticesArray.Size()), Storage);

	VertexEntity NewVertex;
	for (const auto &DomVertex : DomVerticesArray) {
		if (!VertexCommands::ConstFuncs::Deserialize(
			Storage,
			GraphId,
			DomVertex,
			ErrorMessage,
			NewVertex
		)) {
			ExecuteSubCommand(Remove(GraphId), Storage);
			return false;
		}
		ExecuteSubCommand(VertexCommands::Create(
			GraphId, nullptr,
			NewVertex.UserId,
			NewVertex.Position,
			NewVertex.Color
		), Storage);
	}

	const auto &EdgesMember = DomGraph.FindMember("edges");
	if (EdgesMember == DomGraph.MemberEnd())
		return true;

	if (!EdgesMember->value.IsArray()) {
		ErrorMessage = "\"edges\" should be an array of objects.";
		ExecuteSubCommand(Remove(GraphId), Storage);
		return false;
	}
	const auto &DomEdgesArray = EdgesMember->value.GetArray();
	if (DomEdgesArray.Size() == 0)
		return true;

	ExecuteSubCommand(EdgeCommands::Reserve(GraphId, DomEdgesArray.Size()), Storage);

	EdgeEntity NewEdge;
	for (const auto &DomEdge : DomEdgesArray) {
		if (!EdgeCommands::ConstFuncs::Deserialize(
			Storage,
			GraphId,
			DomEdge,
			ErrorMessage,
			NewEdge
		)) {
			ExecuteSubCommand(Remove(GraphId), Storage);
			return false;
		}
		ExecuteSubCommand(EdgeCommands::Create(
			GraphId, nullptr,
			NewEdge.VerticesIds[0], NewEdge.VerticesIds[1]
		), Storage);
	}

	return true;
}) {}

void GraphCommands::ConstFuncs::Serialize(
	const EntityStorage &Storage,
	const EntityId &GraphId,
	rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
) {
	const auto &Graph = Storage.GetEntity<GraphEntity>(GraphId);

	Writer.StartObject();

	Writer.Key("colorful");
	Writer.Bool(Graph.UseDefaultColors);

	check(Graph.VerticesIds.Num() > 0);
	Writer.Key("vertices");
	Writer.StartArray();
	for (const auto &VertexId : Graph.VerticesIds)
		VertexCommands::ConstFuncs::Serialize(Storage, VertexId, Writer);
	Writer.EndArray();

	if (Graph.EdgesIds.Num() > 0) {
		Writer.Key("edges");
		Writer.StartArray();
		for (const auto &EdgeId : Graph.EdgesIds)
			EdgeCommands::ConstFuncs::Serialize(Storage, EdgeId, Writer);
		Writer.EndArray();
	}

	Writer.EndObject();
}

FVector GraphCommands::ConstFuncs::ComputeCenterPosition(const EntityStorage &Storage, const EntityId &GraphId) {
	const auto &Graph = Storage.GetEntity<GraphEntity>(GraphId);
	check(Graph.VerticesIds.Num() > 0);

	FVector Center = FVector::ZeroVector;
	for (const auto &VertexId : Graph.VerticesIds)
		Center += Storage.GetEntity<VertexEntity>(VertexId).Position;
	Center /= Graph.VerticesIds.Num();

	return Center;
}

bool GraphCommands::ConstFuncs::IsSetContainsGraphChildrenEntities(
	const EntityStorage &Storage,
	const EntityId &GraphId,
	const TSet<EntityId> &InSet
) {
	const auto &Graph = Storage.GetEntity<GraphEntity>(GraphId);

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
