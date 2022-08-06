#include "VertexCommands.h"
#include "Graphs/GraphProvider/Entities/GraphEntity.h"
#include "Graphs/GraphProvider/Entities/VertexEntity.h"
#include "Graphs/Utils/Colors.h"

VertexCommands::Create::Create(
	EntityId GraphId,
	EntityId *NewVertexId,
	const FVector &Position
) : Command([GraphId, NewVertexId, &Position] (AGraphProvider &Provider) {
	const auto NewVertex = CreateEntity<VertexEntity>(Provider);

	const auto Graph = dynamic_cast<GraphEntity*>(GetEntity(Provider, GraphId));
	NewVertex->GetActor()->AttachToActor(Graph->GetActor(), FAttachmentTransformRules::KeepWorldTransform);
	Graph->VerticesIds.Push(NewVertex->GetId());

	NewVertex->GraphId = GraphId;
	NewVertex->GetActor()->SetActorLocation(Position);

	if (NewVertexId)
		*NewVertexId = NewVertex->GetId();
}) {}

VertexCommands::Remove::Remove(EntityId Id) : Command([Id] (AGraphProvider &Provider) {
	RemoveEntity(Provider, Id);
}) {}

VertexCommands::GetGraphId::GetGraphId(
	EntityId Id,
	EntityId &GraphId
) : Command([Id, &GraphId] (const AGraphProvider &Provider) {
	const auto Vertex = dynamic_cast<VertexEntity*>(GetEntity(Provider, Id));
	GraphId = Vertex->GraphId;
}) {}

VertexCommands::SetSelectionType::SetSelectionType(
	EntityId Id,
	SelectionType NewType
) : Command([Id, NewType] (const AGraphProvider &Provider) {
	const auto Vertex = dynamic_cast<VertexEntity*>(GetEntity(Provider, Id));
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
	const FVector &Delta
) : Command([Id, &Delta] (const AGraphProvider &Provider) {
	const auto Vertex = dynamic_cast<VertexEntity*>(GetEntity(Provider, Id));
	Vertex->GetActor()->SetActorLocation(Vertex->GetActor()->GetActorLocation() + Delta);
}) {}
