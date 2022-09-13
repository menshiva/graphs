﻿#include "VertexCommands.h"
#include "EdgeCommands.h"
#include "GraphCommands.h"

VertexCommands::Create::Create(
	const EntityId &GraphId, EntityId *NewVertexId,
	const uint32_t UserId,
	const FVector &Position,
	const FColor &Color
) : Command([&, NewVertexId, UserId] (EntityStorage &Storage) -> bool {
	const auto VertexId = Storage.NewEntity<VertexEntity>();
	auto &Vertex = Storage.GetEntityMut<VertexEntity>(VertexId);

	Vertex.GraphId = GraphId;
	Vertex.IsHit = false;
	Vertex.OverrideColor = ColorConsts::OverrideColorNone;

	Vertex.UserId = UserId;
	Vertex.Position = Position;
	Vertex.Color = Color;

	auto &ParentGraph = Storage.GetEntityMut<GraphEntity>(GraphId);
	check(!ParentGraph.VertexUserIdToEntityId.Contains(UserId));
	ParentGraph.VertexUserIdToEntityId.Add(UserId, VertexId);
	check(!ParentGraph.VerticesIds.Contains(VertexId));
	ParentGraph.VerticesIds.Push(VertexId);

	if (NewVertexId)
		*NewVertexId = VertexId;

	return true;
}) {}

VertexCommands::Remove::Remove(const EntityId &VertexId) : Command([&] (EntityStorage &Storage) -> bool {
	auto &Vertex = Storage.GetEntityMut<VertexEntity>(VertexId);

	// remove from associated parent graph
	auto &Graph = Storage.GetEntityMut<GraphEntity>(Vertex.GraphId);
	auto CheckNum = Graph.VerticesIds.Remove(VertexId);
	check(CheckNum == 1);

	if (Graph.VerticesIds.Num() > 0) {
		CheckNum = Graph.VertexUserIdToEntityId.Remove(Vertex.UserId);
		check(CheckNum == 1);

		// move connected edges ids because otherwise the next
		// EdgeCommands::Remove command will invalidate EdgesIds while we iterating it
		const auto EdgeIds(MoveTemp(Vertex.EdgesIds));
		for (const auto &EdgeId : EdgeIds)
			ExecuteSubCommand(EdgeCommands::Remove(EdgeId), Storage);
	}
	else {
		ExecuteSubCommand(GraphCommands::Remove(Vertex.GraphId), Storage);
	}

	Storage.RemoveEntity<VertexEntity>(VertexId);
	return true;
}) {}

VertexCommands::Reserve::Reserve(
	const EntityId &GraphId,
	const uint32_t NewVerticesNum
) : Command([&, NewVerticesNum] (EntityStorage &Storage) -> bool {
	auto &VerticesStorage = Storage.GetStorageMut<VertexEntity>().Data;
	VerticesStorage.Reserve(VerticesStorage.Num() + NewVerticesNum);

	auto &Graph = Storage.GetEntityMut<GraphEntity>(GraphId);
	Graph.VertexUserIdToEntityId.Reserve(Graph.VertexUserIdToEntityId.Num() + NewVerticesNum);
	Graph.VerticesIds.Reserve(Graph.VerticesIds.Num() + NewVerticesNum);

	return true;
}) {}

VertexCommands::SetHit::SetHit(
	const EntityId &VertexId,
	const bool IsHit
) : Command([&, IsHit] (EntityStorage &Storage) -> bool {
	auto &Vertex = Storage.GetEntityMut<VertexEntity>(VertexId);

	if (Vertex.IsHit == IsHit)
		return false;
	Vertex.IsHit = IsHit;

	return true;
}) {}

VertexCommands::SetOverrideColor::SetOverrideColor(
	const EntityId &VertexId,
	const FColor &OverrideColor
) : Command([&] (EntityStorage &Storage) -> bool {
	auto &Vertex = Storage.GetEntityMut<VertexEntity>(VertexId);

	if (Vertex.OverrideColor == OverrideColor)
		return false;
	Vertex.OverrideColor = OverrideColor;

	return true;
}) {}

VertexCommands::Move::Move(
	const EntityId &VertexId,
	const FVector &Delta
) : Command([&] (EntityStorage &Storage) -> bool {
	Storage.GetEntityMut<VertexEntity>(VertexId).Position += Delta;
	return true;
}) {}

void VertexCommands::ConstFuncs::Serialize(
	const EntityStorage &Storage,
	const EntityId &VertexId,
	rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
) {
	const auto &Vertex = Storage.GetEntity<VertexEntity>(VertexId);
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

bool VertexCommands::ConstFuncs::Deserialize(
	const EntityStorage &Storage,
	const EntityId &GraphId,
	const rapidjson::Value &DomVertex,
	FString &ErrorMessage,
	VertexEntity &NewVertex
) {
	if (!DomVertex.IsObject()) {
		ErrorMessage = "Vertex error: Should be an object.";
		return false;
	}

	const auto &ParentGraph = Storage.GetEntity<GraphEntity>(GraphId);

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
