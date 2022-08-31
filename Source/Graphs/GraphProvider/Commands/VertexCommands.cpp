#include "VertexCommands.h"
#include "EdgeCommands.h"
#include "Graphs/GraphProvider/Entities/GraphEntity.h"
#include "Graphs/GraphProvider/Entities/VertexEntity.h"
#include "Graphs/Utils/Consts.h"

VertexCommands::Create::Create(
	EntityId GraphId,
	EntityId *NewVertexId,
	uint32_t VertexDisplayId,
	const FVector &Position
) : Command([GraphId, NewVertexId, VertexDisplayId, &Position] (AGraphProvider &Provider) {
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

VertexCommands::Remove::Remove(EntityId Id) : Command([Id] (AGraphProvider &Provider) {
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

VertexCommands::GetGraphId::GetGraphId(
	EntityId Id,
	EntityId &GraphId
) : Command([Id, &GraphId] (const AGraphProvider &Provider) {
	GraphId = GetEntity<const VertexEntity>(Provider, Id)->GraphId;
}) {}

VertexCommands::SetColor::SetColor(
	EntityId Id,
	const FLinearColor &Color
) : Command([Id, &Color] (const AGraphProvider &Provider) {
	const auto Vertex = GetEntity<VertexEntity>(Provider, Id);
	Vertex->SetActorColor(Color);
}) {}

VertexCommands::SetSelectionType::SetSelectionType(
	EntityId Id,
	SelectionType NewType
) : Command([Id, NewType] (AGraphProvider &Provider) {
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

VertexCommands::Move::Move(
	EntityId Id,
	const FVector &Delta,
	bool UpdateConnectedEdges
) : Command([Id, &Delta, UpdateConnectedEdges] (AGraphProvider &Provider) {
	const auto Vertex = GetEntity<const VertexEntity>(Provider, Id);

	Vertex->Actor->SetActorLocation(Vertex->Actor->GetActorLocation() + Delta);

	if (UpdateConnectedEdges)
		for (const auto EdgeId : Vertex->EdgesIds)
			Provider.ExecuteCommand(EdgeCommands::UpdateTransform(EdgeId));
}) {}

VertexCommands::Serialize::Serialize(
	EntityId Id,
	rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
) : Command([Id, &Writer] (const AGraphProvider &Provider) {
	const auto Vertex = GetEntity<const VertexEntity>(Provider, Id);

	Writer.StartObject();

	Writer.Key("id");
	Writer.Uint(Vertex->DisplayId);

	const auto Pos = Vertex->Actor->GetActorLocation();
	Writer.Key("position");
	Writer.StartObject();
	Writer.Key("x");
	Writer.Double(Pos.X);
	Writer.Key("y");
	Writer.Double(Pos.Y);
	Writer.Key("z");
	Writer.Double(Pos.Z);
	Writer.EndObject();

	Writer.EndObject();
}) {}

VertexCommands::Deserialize::Deserialize(
	EntityId GraphId,
	EntityId *NewVertexId,
	rapidjson::Value &VertexDomValue,
	FString &ErrorMessage
) : Command([GraphId, NewVertexId, &VertexDomValue, &ErrorMessage] (AGraphProvider &Provider) {
	*NewVertexId = ENTITY_NONE;

	const auto &IdMember = VertexDomValue.FindMember("id");
	if (IdMember == VertexDomValue.MemberEnd() || !IdMember->value.IsUint()) {
		ErrorMessage = "Vertex should have an integer \"id\" field.";
		return;
	}
	const uint32_t NewVertexDisplayId = IdMember->value.GetUint();

	const auto &PositionMember = VertexDomValue.FindMember("position");
	if (PositionMember == VertexDomValue.MemberEnd() || !PositionMember->value.IsObject()) {
		ErrorMessage = "Vertex with id "
			+ FString::FromInt(NewVertexDisplayId)
			+ " should have\n\"position\" object with X, Y, Z fields.";
		return;
	}
	const auto &PositionObject = PositionMember->value.GetObject();
	const auto &PositionXMember = PositionObject.FindMember("x");
	const auto &PositionYMember = PositionObject.FindMember("y");
	const auto &PositionZMember = PositionObject.FindMember("z");
	for (const auto &PosIter : {PositionXMember, PositionYMember, PositionZMember}) {
		if (PosIter == PositionObject.MemberEnd() || !PosIter->value.IsFloat()) {
			ErrorMessage = "Vertex with id "
				+ FString::FromInt(NewVertexDisplayId)
				+ " should have\n\"position\" object with X, Y, Z fields.";
			return;
		}
	}
	const FVector NewVertexPosition(
		PositionXMember->value.GetFloat(),
		PositionYMember->value.GetFloat(),
		PositionZMember->value.GetFloat()
	);

	Provider.ExecuteCommand(Create(GraphId, NewVertexId, NewVertexDisplayId, NewVertexPosition));
}) {}
