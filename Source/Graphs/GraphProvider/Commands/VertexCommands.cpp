#include "VertexCommands.h"
#include "EdgeCommands.h"
#include "Graphs/GraphProvider/Entities/GraphEntity.h"
#include "Graphs/GraphProvider/Entities/VertexEntity.h"
#include "Graphs/Utils/Colors.h"

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

VertexCommands::GetGraphId::GetGraphId(
	EntityId Id,
	EntityId &GraphId
) : Command([Id, &GraphId] (const AGraphProvider &Provider) {
	GraphId = GetEntity<const VertexEntity>(Provider, Id)->GraphId;
}) {}

VertexCommands::SetSelectionType::SetSelectionType(
	EntityId Id,
	SelectionType NewType
) : Command([Id, NewType] (const AGraphProvider &Provider) {
	const auto Vertex = GetEntity<VertexEntity>(Provider, Id);
	Vertex->Selection = NewType;
	switch (NewType) {
		case SelectionType::HIT:
		case SelectionType::SELECTED: {
			Vertex->SetActorColor(ColorUtils::BlueColor);
			break;
		}
		default: {
			Vertex->SetActorColor(ColorUtils::GraphDefaultColor);
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
