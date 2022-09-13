﻿#include "VertexCommands.h"
#include "EdgeCommands.h"
#include "Graphs/GraphProvider/Entities/GraphEntity.h"
#include "Graphs/GraphProvider/Entities/VertexEntity.h"
#include "Graphs/Utils/Consts.h"

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Create"), STAT_VertexCommands_Create, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::Create::Create(
	EntityId GraphId,
	EntityId *NewVertexId,
	uint32_t VertexDisplayId,
	const FVector &Position
) : Command([GraphId, NewVertexId, VertexDisplayId, &Position] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Create);
	const auto NewVertex = CreateEntity<VertexEntity>(Provider);

	const auto Graph = GetEntity<GraphEntity>(Provider, GraphId);
	NewVertex->Actor->AttachToActor(Graph->Actor.Get(), FAttachmentTransformRules::KeepWorldTransform);
	check(!Graph->VerticesIds.Contains(NewVertex->GetEntityId()));
	Graph->VerticesIds.Push(NewVertex->GetEntityId());

	NewVertex->GraphId = GraphId;
	NewVertex->DisplayId = VertexDisplayId;
	NewVertex->Actor->SetActorLocation(Position);

	if (NewVertexId)
		*NewVertexId = NewVertex->GetEntityId();
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Remove"), STAT_VertexCommands_Remove, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::Remove::Remove(EntityId Id) : Command([Id] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Remove);
	const auto Vertex = GetEntity<VertexEntity>(Provider, Id);

	// EdgeCommands::Remove command will remove entities from Vertex->EdgesIds TArray.
	// This will cause a problem while we iterating this TArray here, so we move all TArray data.
	const TArray EdgeIds(MoveTemp(Vertex->EdgesIds));
	for (const auto EdgeId : EdgeIds)
		Provider.ExecuteCommand(EdgeCommands::Remove(EdgeId));

	const auto Graph = GetEntity<GraphEntity>(Provider, Vertex->GraphId);
	Graph->VerticesIds.RemoveSingle(Id);
	check(!Graph->VerticesIds.Contains(Id));

	RemoveEntity(Provider, Id);
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::GetGraphId"), STAT_VertexCommands_GetGraphId, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::GetGraphId::GetGraphId(
	EntityId Id,
	EntityId &GraphId
) : Command([Id, &GraphId] (const AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_GetGraphId);
	GraphId = GetEntity<const VertexEntity>(Provider, Id)->GraphId;
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::SetColor"), STAT_VertexCommands_SetColor, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::SetColor::SetColor(
	EntityId Id,
	const FLinearColor &Color
) : Command([Id, &Color] (const AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_SetColor);
	GetEntity<VertexEntity>(Provider, Id)->SetActorColor(Color);
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::SetSelectionType"), STAT_VertexCommands_SetSelectionType, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::SetSelectionType::SetSelectionType(
	EntityId Id,
	SelectionType NewType
) : Command([Id, NewType] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_SetSelectionType);
	const auto Vertex = GetEntity<VertexEntity>(Provider, Id);
	Vertex->Selection = NewType;

	switch (NewType) {
		case SelectionType::HIT:
		case SelectionType::SELECTED: {
			Provider.ExecuteCommand(SetColor(Id, ColorConsts::BlueColor));
			break;
		}
		default: {
			Provider.ExecuteCommand(SetColor(Id, ColorConsts::GraphDefaultColor));
		}
	}
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Move"), STAT_VertexCommands_Move, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::Move::Move(
	EntityId Id,
	const FVector &Delta,
	bool UpdateConnectedEdges
) : Command([Id, &Delta, UpdateConnectedEdges] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Move);
	const auto Vertex = GetEntity<const VertexEntity>(Provider, Id);

	Vertex->Actor->SetActorLocation(Vertex->Actor->GetActorLocation() + Delta);

	if (UpdateConnectedEdges)
		for (const auto EdgeId : Vertex->EdgesIds)
			Provider.ExecuteCommand(EdgeCommands::UpdateTransform(EdgeId));
}) {}

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Serialize"), STAT_VertexCommands_Serialize, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::Serialize::Serialize(
	EntityId Id,
	rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
) : Command([Id, &Writer] (const AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Serialize);
	const auto Vertex = GetEntity<const VertexEntity>(Provider, Id);
	const auto Pos = Vertex->Actor->GetActorLocation();

	Writer.StartObject();
	Writer.Key("id");
	Writer.Uint(Vertex->DisplayId);
	Writer.Key("x");
	Writer.Double(Pos.X);
	Writer.Key("y");
	Writer.Double(Pos.Y);
	Writer.Key("z");
	Writer.Double(Pos.Z);
	Writer.EndObject();
}) {}

struct VertexSAXDeserializationHandler {
	VertexSAXDeserializationHandler() = default;
	VertexSAXDeserializationHandler(const VertexSAXDeserializationHandler&) = delete;
	VertexSAXDeserializationHandler& operator=(const VertexSAXDeserializationHandler&) = delete;

	enum class State {
		VERTEX,
		ID_KEY,
		X_KEY,
		Y_KEY,
		Z_KEY,
		DONE
	};

	static bool Null() { return false; }
	static bool Bool(bool) { return false; }
	static bool RawNumber(const char*, rapidjson::SizeType, bool) { return false; }
	static bool String(const char*, rapidjson::SizeType, bool) { return false; }
	static bool StartObject() { return false; }
	static bool StartArray() { return false; }
	static bool EndArray(rapidjson::SizeType) { return false; }
	static bool Int(int) { return false; }
	static bool Int64(int64_t) { return false; }
	static bool Uint64(uint64_t) { return false; }

	bool Key(const char *Str, rapidjson::SizeType, bool) {
		if (CurrentState == State::VERTEX) {
			if (strcmp(Str, "id") == 0) {
				CurrentState = State::ID_KEY;
				return true;
			}
			if (strcmp(Str, "x") == 0) {
				CurrentState = State::X_KEY;
				return true;
			}
			if (strcmp(Str, "y") == 0) {
				CurrentState = State::Y_KEY;
				return true;
			}
			if (strcmp(Str, "z") == 0) {
				CurrentState = State::Z_KEY;
				return true;
			}
		}
		return false;
	}

	bool EndObject(rapidjson::SizeType) {
		if (CurrentState == State::VERTEX && Id != -1 && !Pos.ContainsNaN()) {
			CurrentState = State::DONE;
			return true;
		}
		return false;
	}

	bool Uint(const unsigned Val) {
		if (CurrentState == State::ID_KEY) {
			Id = Val;
			CurrentState = State::VERTEX;
			return true;
		}
		return false;
	}

	bool Double(const double Val) {
		if (CurrentState == State::X_KEY) {
			Pos.X = Val;
			CurrentState = State::VERTEX;
			return true;
		}
		if (CurrentState == State::Y_KEY) {
			Pos.Y = Val;
			CurrentState = State::VERTEX;
			return true;
		}
		if (CurrentState == State::Z_KEY) {
			Pos.Z = Val;
			CurrentState = State::VERTEX;
			return true;
		}
		return false;
	}

	State CurrentState = State::VERTEX;
	uint32_t Id = -1;
	FVector Pos = FVector(NAN);
};

DECLARE_CYCLE_STAT(TEXT("VertexCommands::Deserialize"), STAT_VertexCommands_Deserialize, STATGROUP_GRAPHS_PERF_COMMANDS);
VertexCommands::Deserialize::Deserialize(
	EntityId GraphId,
	EntityId *NewVertexId,
	rapidjson::StringStream &JsonStringStream,
	rapidjson::Reader &Reader
) : Command([GraphId, NewVertexId, &JsonStringStream, &Reader] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_VertexCommands_Deserialize);
	*NewVertexId = ENTITY_NONE;

	VertexSAXDeserializationHandler Handler;
	while (!Reader.IterativeParseComplete()) {
		if (!Reader.IterativeParseNext<rapidjson::kParseDefaultFlags>(JsonStringStream, Handler))
			return;
		if (Handler.CurrentState == VertexSAXDeserializationHandler::State::DONE)
			break;
	}

	if (Handler.CurrentState == VertexSAXDeserializationHandler::State::DONE) {
		Provider.ExecuteCommand(Create(
			GraphId,
			NewVertexId,
			Handler.Id,
			Handler.Pos
		));
	}
}) {}
