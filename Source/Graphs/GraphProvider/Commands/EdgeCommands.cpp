#include "EdgeCommands.h"
#include "VertexCommands.h"
#include "Graphs/GraphProvider/Entities/EdgeEntity.h"
#include "Graphs/GraphProvider/Entities/GraphEntity.h"
#include "Graphs/GraphProvider/Entities/VertexEntity.h"
#include "Graphs/Utils/Consts.h"

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Create"), STAT_EdgeCommands_Create, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Create::Create(
	EntityId GraphId,
	EntityId *NewEdgeId,
	EntityId FromVertexId, EntityId ToVertexId
) : Command([GraphId, NewEdgeId, FromVertexId, ToVertexId] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Create);

	const auto NewEdge = CreateEntity<EdgeEntity>(Provider);

	const auto Graph = GetEntity<GraphEntity>(Provider, GraphId);
	NewEdge->Actor->AttachToActor(Graph->Actor.Get(), FAttachmentTransformRules::KeepWorldTransform);
	check(!Graph->EdgesIds.Contains(NewEdge->GetEntityId()));
	Graph->EdgesIds.Push(NewEdge->GetEntityId());

	NewEdge->GraphId = GraphId;
	NewEdge->VerticesIds[0] = FromVertexId;
	NewEdge->VerticesIds[1] = ToVertexId;

	for (const auto VertexId : NewEdge->VerticesIds) {
		const auto Vertex = GetEntity<VertexEntity>(Provider, VertexId);
		check(!Vertex->EdgesIds.Contains(NewEdge->GetEntityId()));
		Vertex->EdgesIds.Push(NewEdge->GetEntityId());
	}

	Provider.ExecuteCommand(UpdateTransform(NewEdge->GetEntityId()));

	if (NewEdgeId)
		*NewEdgeId = NewEdge->GetEntityId();
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Remove"), STAT_EdgeCommands_Remove, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Remove::Remove(EntityId Id) : Command([Id] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Remove);
	const auto Edge = GetEntity<const EdgeEntity>(Provider, Id);

	for (const auto VertexId : Edge->VerticesIds) {
		const auto Vertex = GetEntity<VertexEntity>(Provider, VertexId);
		Vertex->EdgesIds.RemoveSingle(Id);
		check(!Vertex->EdgesIds.Contains(Id));
	}

	const auto Graph = GetEntity<GraphEntity>(Provider, Edge->GraphId);
	Graph->EdgesIds.RemoveSingle(Id);
	check(!Graph->EdgesIds.Contains(Id));

	RemoveEntity(Provider, Id);
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::GetGraphId"), STAT_EdgeCommands_GetGraphId, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::GetGraphId::GetGraphId(
	EntityId Id,
	EntityId &GraphId
) : Command([Id, &GraphId] (const AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_GetGraphId);
	GraphId = GetEntity<const EdgeEntity>(Provider, Id)->GraphId;
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::UpdateTransform"), STAT_EdgeCommands_UpdateTransform, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::UpdateTransform::UpdateTransform(EntityId Id) : Command([Id] (const AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_UpdateTransform);

	const auto Edge = GetEntity<const EdgeEntity>(Provider, Id);
	const auto FirstVertexPos = GetEntity<const VertexEntity>(Provider, Edge->VerticesIds[0])->Actor->GetActorLocation();
	const auto SecondVertexPos = GetEntity<const VertexEntity>(Provider, Edge->VerticesIds[1])->Actor->GetActorLocation();

	const auto Dir = (SecondVertexPos - FirstVertexPos).GetSafeNormal();
	Edge->Actor->SetActorLocationAndRotation(
		(FirstVertexPos + SecondVertexPos) / 2.0f,
		FQuat::FindBetweenNormals(FVector::UpVector, Dir.GetSafeNormal())
	);

	auto Scale = Edge->Actor->GetActorScale3D();
	Scale.Z = FVector::Dist(FirstVertexPos, SecondVertexPos) / 100.0f;
	Edge->Actor->SetActorScale3D(Scale);
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::SetColor"), STAT_EdgeCommands_SetColor, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::SetColor::SetColor(
	EntityId Id,
	const FLinearColor &Color
) : Command([Id, &Color] (const AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_SetColor);
	GetEntity<EdgeEntity>(Provider, Id)->SetActorColor(Color);
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::SetSelectionType"), STAT_EdgeCommands_SetSelectionType, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::SetSelectionType::SetSelectionType(
	EntityId Id,
	SelectionType NewType
) : Command([Id, NewType] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_SetSelectionType);

	const auto Edge = GetEntity<EdgeEntity>(Provider, Id);
	Edge->Selection = NewType;

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

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Move"), STAT_EdgeCommands_Move, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Move::Move(
	EntityId Id,
	const FVector& Delta,
	bool UpdateConnectedVertices
) : Command([Id, &Delta, UpdateConnectedVertices] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Move);
	const auto Edge = GetEntity<const EdgeEntity>(Provider, Id);

	if (UpdateConnectedVertices)
		for (const auto VertexId : Edge->VerticesIds)
			Provider.ExecuteCommand(VertexCommands::Move(VertexId, Delta, true));
	else
		Edge->Actor->SetActorLocation(Edge->Actor->GetActorLocation() + Delta);
}) {}

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Serialize"), STAT_EdgeCommands_Serialize, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Serialize::Serialize(
	EntityId Id,
	rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
) : Command([Id, &Writer] (const AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Serialize);

	const auto Edge = GetEntity<const EdgeEntity>(Provider, Id);
	const auto FromVertex = GetEntity<const VertexEntity>(Provider, Edge->VerticesIds[0]);
	const auto ToVertex = GetEntity<const VertexEntity>(Provider, Edge->VerticesIds[1]);

	Writer.StartObject();
	Writer.Key("from_vertex_id");
	Writer.Uint(FromVertex->DisplayId);
	Writer.Key("to_vertex_id");
	Writer.Uint(ToVertex->DisplayId);
	Writer.EndObject();
}) {}

struct EdgeSAXDeserializationHandler {
	EdgeSAXDeserializationHandler() = default;
	EdgeSAXDeserializationHandler(const EdgeSAXDeserializationHandler&) = delete;
	EdgeSAXDeserializationHandler& operator=(const EdgeSAXDeserializationHandler&) = delete;

	enum class State {
		EDGE,
		FROM_VERTEX_ID_KEY,
		TO_VERTEX_ID_KEY,
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
	static bool Double(double) { return false; }

	bool Key(const char *Str, rapidjson::SizeType, bool) {
		if (CurrentState == State::EDGE) {
			if (strcmp(Str, "from_vertex_id") == 0) {
				CurrentState = State::FROM_VERTEX_ID_KEY;
				return true;
			}
			if (strcmp(Str, "to_vertex_id") == 0) {
				CurrentState = State::TO_VERTEX_ID_KEY;
				return true;
			}
		}
		return false;
	}

	bool EndObject(rapidjson::SizeType) {
		if (CurrentState == State::EDGE && FromVertexId != -1 && ToVertexId != -1) {
			CurrentState = State::DONE;
			return true;
		}
		return false;
	}

	bool Uint(const unsigned Val) {
		if (CurrentState == State::FROM_VERTEX_ID_KEY) {
			FromVertexId = Val;
			CurrentState = State::EDGE;
			return true;
		}
		if (CurrentState == State::TO_VERTEX_ID_KEY) {
			ToVertexId = Val;
			CurrentState = State::EDGE;
			return true;
		}
		return false;
	}

	State CurrentState = State::EDGE;
	uint32_t FromVertexId = -1, ToVertexId = -1;
};

DECLARE_CYCLE_STAT(TEXT("EdgeCommands::Deserialize"), STAT_EdgeCommands_Deserialize, STATGROUP_GRAPHS_PERF_COMMANDS);
EdgeCommands::Deserialize::Deserialize(
	EntityId GraphId,
	EntityId *NewEdgeId,
	rapidjson::StringStream &JsonStringStream,
	rapidjson::Reader &Reader,
	const TMap<uint32_t, EntityId> &VerticesIdsMapping,
	FString &ErrorMessage
) : Command([GraphId, NewEdgeId, &JsonStringStream, &Reader, &VerticesIdsMapping, &ErrorMessage] (AGraphProvider &Provider) {
	SCOPE_CYCLE_COUNTER(STAT_EdgeCommands_Deserialize);

	*NewEdgeId = ENTITY_NONE;

	EdgeSAXDeserializationHandler Handler;
	while (!Reader.IterativeParseComplete()) {
		if (!Reader.IterativeParseNext<rapidjson::kParseDefaultFlags>(JsonStringStream, Handler))
			return;
		if (Handler.CurrentState == EdgeSAXDeserializationHandler::State::DONE)
			break;
	}

	if (Handler.CurrentState == EdgeSAXDeserializationHandler::State::DONE) {
		const auto FromIdVertexMapping = VerticesIdsMapping.Find(Handler.FromVertexId);
		if (!FromIdVertexMapping) {
			ErrorMessage = "Vertex with id from \"from_id\" field is not found.";
			return;
		}
		const auto ToIdVertexMapping = VerticesIdsMapping.Find(Handler.ToVertexId);
		if (!ToIdVertexMapping) {
			ErrorMessage = "Vertex with id from \"to_id\" field is not found.";
			return;
		}

		Provider.ExecuteCommand(Create(GraphId, NewEdgeId, *FromIdVertexMapping, *ToIdVertexMapping));
	}
}) {}
