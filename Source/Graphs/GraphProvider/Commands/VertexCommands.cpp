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
	NewVertex->GraphId = GraphId;
	NewVertex->GetActor()->SetActorLocation(Position);
	const auto Graph = dynamic_cast<GraphEntity*>(GetMutEntity(Provider, GraphId));
	Graph->VerticesIds.Push(NewVertex->GetId());
	if (NewVertexId)
		*NewVertexId = NewVertex->GetId();
}) {}

VertexCommands::Remove::Remove(EntityId Id) : Command([Id] (AGraphProvider &Provider) {
	RemoveEntity(Provider, Id);
}) {}

VertexCommands::SetSelectionType::SetSelectionType(
	EntityId Id,
	SelectionType NewType
) : Command([Id, NewType] (const AGraphProvider &Provider) {
	const auto Vertex = dynamic_cast<VertexEntity*>(GetMutEntity(Provider, Id));
	Vertex->Selection = NewType;
	switch (NewType) {
		case SelectionType::HIT:
		case SelectionType::SELECTED: {
			Vertex->SetActorColor(ColorUtils::SelectionColor);
			break;
		}
		default: {
			Vertex->SetActorColor(ColorUtils::GraphDefaultColor);
		}
	}
}) {}
