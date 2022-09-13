#include "GraphCommands.h"
#include "EdgeCommands.h"
#include "VertexCommands.h"
#include "Graphs/GraphProvider/Entities/EdgeEntity.h"
#include "Graphs/GraphProvider/Entities/GraphEntity.h"
#include "Graphs/GraphProvider/Entities/VertexEntity.h"

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Create"), STAT_GraphCommands_Create, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::Create::Create(
	EntityId *NewGraphId
) : Command([NewGraphId] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Create);
	const auto NewNode = CreateEntity<GraphEntity>(Provider);
	if (NewGraphId)
		*NewGraphId = NewNode->GetEntityId();
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Remove"), STAT_GraphCommands_Remove, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::Remove::Remove(EntityId Id) : Command([Id] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Remove);
	const auto Graph = GetEntity<GraphEntity>(Provider, Id);

	for (const auto VertexId : Graph->VerticesIds)
		RemoveEntity(Provider, VertexId);
	for (const auto EdgeId : Graph->EdgesIds)
		RemoveEntity(Provider, EdgeId);

	RemoveEntity(Provider, Id);
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::SetColor"), STAT_GraphCommands_SetColor, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::SetColor::SetColor(
	EntityId Id,
	const FLinearColor &Color
) : Command([Id, &Color] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_SetColor);
	const auto Graph = GetEntity<GraphEntity>(Provider, Id);

	for (const auto VertexId : Graph->VerticesIds)
		Provider.ExecuteCommand(VertexCommands::SetColor(VertexId, Color));

	for (const auto EdgeId : Graph->EdgesIds)
		Provider.ExecuteCommand(EdgeCommands::SetColor(EdgeId, Color));
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::SetSelectionType"), STAT_GraphCommands_SetSelectionType, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::SetSelectionType::SetSelectionType(
	EntityId Id,
	SelectionType NewType
) : Command([Id, NewType] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_SetSelectionType);
	const auto Graph = GetEntity<GraphEntity>(Provider, Id);

	for (const auto VertexId : Graph->VerticesIds)
		Provider.ExecuteCommand(VertexCommands::SetSelectionType(VertexId, NewType));

	for (const auto EdgeId : Graph->EdgesIds)
		Provider.ExecuteCommand(EdgeCommands::SetSelectionType(EdgeId, NewType));
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Move"), STAT_GraphCommands_Move, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::Move::Move(
	EntityId Id,
	const FVector &Delta
) : Command([Id, &Delta] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Move);
	const auto Graph = GetEntity<const GraphEntity>(Provider, Id);

	for (const auto VertexId : Graph->VerticesIds)
		Provider.ExecuteCommand(VertexCommands::Move(VertexId, Delta, false));

	for (const auto EdgeId : Graph->EdgesIds)
		Provider.ExecuteCommand(EdgeCommands::Move(EdgeId, Delta, false));
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::ComputeCenterPosition"), STAT_GraphCommands_ComputeCenterPosition, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::ComputeCenterPosition::ComputeCenterPosition(
	EntityId Id,
	FVector &Center
) : Command([Id, &Center] (const AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_ComputeCenterPosition);
	const auto Graph = GetEntity<const GraphEntity>(Provider, Id);
	check(Graph->VerticesIds.Num() > 0);

	Center = FVector::ZeroVector;
	for (const auto VertexId : Graph->VerticesIds)
		Center += GetEntity<const VertexEntity>(Provider, VertexId)->Actor->GetActorLocation();
	Center /= Graph->VerticesIds.Num();
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Rotate"), STAT_GraphCommands_Rotate, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::Rotate::Rotate(
	EntityId Id,
	const FVector &Center,
	const float Angle
) : Command([Id, &Center, Angle] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Rotate);
	const auto Graph = GetEntity<const GraphEntity>(Provider, Id);

	for (const auto VertexId : Graph->VerticesIds) {
		const auto Vertex = GetEntity<const VertexEntity>(Provider, VertexId);

		const auto VertexPos = Vertex->Actor->GetActorLocation();
		const auto PosDirection = VertexPos - Center;
		const auto RotatedPos = PosDirection.RotateAngleAxis(Angle, FVector::DownVector) + Center;

		Provider.ExecuteCommand(VertexCommands::Move(VertexId, RotatedPos - VertexPos, false));
	}

	for (const auto EdgeId : Graph->EdgesIds)
		Provider.ExecuteCommand(EdgeCommands::UpdateTransform(EdgeId));
}) {}

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Serialize"), STAT_GraphCommands_Serialize, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::Serialize::Serialize(
	EntityId Id,
	rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
) : Command([Id, &Writer] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Serialize);
	const auto Graph = GetEntity<const GraphEntity>(Provider, Id);

	Writer.StartObject();

	check(Graph->VerticesIds.Num() > 0);
	Writer.Key("vertices");
	Writer.StartArray();
	for (const auto VertexId : Graph->VerticesIds)
		Provider.ExecuteCommand(VertexCommands::Serialize(VertexId, Writer));
	Writer.EndArray();

	if (Graph->EdgesIds.Num() > 0) {
		Writer.Key("edges");
		Writer.StartArray();
		for (const auto EdgeId : Graph->EdgesIds)
			Provider.ExecuteCommand(EdgeCommands::Serialize(EdgeId, Writer));
		Writer.EndArray();
	}

	Writer.EndObject();
}) {}

struct GraphSAXDeserializationHandler {
	GraphSAXDeserializationHandler() = default;
	GraphSAXDeserializationHandler(const GraphSAXDeserializationHandler&) = delete;
	GraphSAXDeserializationHandler &operator=(const GraphSAXDeserializationHandler&) = delete;

	enum class State {
		UNDEF,
		GRAPH,
		VERTICES_KEY,
		VERTICES_ARR,
		VERTEX,
		EDGES_KEY,
		EDGES_ARR,
		EDGE,
		DONE
	};

	static bool Null() { return false; }
	static bool Bool(bool) { return false; }
	static bool Int(int) { return false; }
	static bool Uint(unsigned) { return false; }
	static bool Int64(int64_t) { return false; }
	static bool Uint64(uint64_t) { return false; }
	static bool Double(double) { return false; }
	static bool RawNumber(const char*, rapidjson::SizeType, bool) { return false; }
	static bool String(const char*, rapidjson::SizeType, bool) { return false; }

	bool Key(const char *Str, rapidjson::SizeType, bool) {
		if (CurrentState == State::GRAPH) {
			if (strcmp(Str, "vertices") == 0) {
				CurrentState = State::VERTICES_KEY;
				return true;
			}
			if (strcmp(Str, "edges") == 0) {
				CurrentState = State::EDGES_KEY;
				return true;
			}
		}
		return false;
	}

	bool StartObject() {
		if (CurrentState == State::VERTEX || CurrentState == State::EDGE)
			return true;
		if (CurrentState == State::VERTICES_ARR) {
			CurrentState = State::VERTEX;
			return true;
		}
		if (CurrentState == State::EDGES_ARR) {
			CurrentState = State::EDGE;
			return true;
		}
		if (CurrentState == State::UNDEF) {
			CurrentState = State::GRAPH;
			return true;
		}
		return false;
	}

	bool EndObject(rapidjson::SizeType) {
		if (CurrentState == State::GRAPH) {
			CurrentState = State::DONE;
			return true;
		}
		return false;
	}

	bool StartArray() {
		if (CurrentState == State::VERTICES_KEY) {
			CurrentState = State::VERTICES_ARR;
			return true;
		}
		if (CurrentState == State::EDGES_KEY) {
			CurrentState = State::EDGES_ARR;
			return true;
		}
		return false;
	}

	bool EndArray(rapidjson::SizeType) {
		if (CurrentState == State::VERTEX || CurrentState == State::EDGE) {
			CurrentState = State::GRAPH;
			return true;
		}
		return false;
	}

	State CurrentState = State::UNDEF;
};

DECLARE_CYCLE_STAT(TEXT("GraphCommands::Deserialize"), STAT_GraphCommands_Deserialize, STATGROUP_GRAPHS_PERF_COMMANDS);
GraphCommands::Deserialize::Deserialize(
	EntityId *NewGraphId,
	rapidjson::StringStream &JsonStringStream,
	rapidjson::Reader &Reader,
	FString &ErrorMessage
) : Command([NewGraphId, &JsonStringStream, &Reader, &ErrorMessage] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_GraphCommands_Deserialize);

	Provider.ExecuteCommand(Create(NewGraphId));
	check(*NewGraphId != ENTITY_NONE);

	TMap<uint32_t, EntityId> VerticesIdsMappings;

	GraphSAXDeserializationHandler Handler;
	while (!Reader.IterativeParseComplete()) {
		if (!Reader.IterativeParseNext<rapidjson::kParseDefaultFlags>(JsonStringStream, Handler))
			break;

		if (Handler.CurrentState == GraphSAXDeserializationHandler::State::VERTEX) {
			EntityId NewVertexId = ENTITY_NONE;
			Provider.ExecuteCommand(VertexCommands::Deserialize(
				*NewGraphId,
				&NewVertexId,
				JsonStringStream,
				Reader
			));
			if (NewVertexId != ENTITY_NONE) {
				const auto NewVertex = GetEntity<const VertexEntity>(Provider, NewVertexId);
				if (VerticesIdsMappings.Contains(NewVertex->DisplayId)) {
					ErrorMessage = "Vertex with id " + FString::FromInt(NewVertex->DisplayId) + "\nis not unique.";
					break;
				}
				VerticesIdsMappings.Add(NewVertex->DisplayId, NewVertexId);
			}
			else break;
		}
		else if (Handler.CurrentState == GraphSAXDeserializationHandler::State::EDGE) {
			EntityId NewEdgeId = ENTITY_NONE;
			Provider.ExecuteCommand(EdgeCommands::Deserialize(
				*NewGraphId,
				&NewEdgeId,
				JsonStringStream,
				Reader,
				VerticesIdsMappings,
				ErrorMessage
			));
			if (NewEdgeId == ENTITY_NONE)
				break;
		}
	}

	if (Handler.CurrentState != GraphSAXDeserializationHandler::State::DONE) {
		Provider.ExecuteCommand(Remove(*NewGraphId));
		*NewGraphId = ENTITY_NONE;
	}
}) {}
