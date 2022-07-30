#include "GraphCommands.h"
#include "VertexCommands.h"
#include "Graphs/GraphProvider/Entities/GraphEntity.h"

GraphCommands::Create::Create(EntityId *NewId) : Command([NewId] (AGraphProvider &Provider) {
	const auto NewNode = CreateEntity<GraphEntity>(Provider);
	if (NewId)
		*NewId = NewNode->GetId();
}) {}

GraphCommands::SetSelectionType::SetSelectionType(
	EntityId Id,
	SelectionType NewType
) : Command([Id, NewType] (AGraphProvider &Provider) {
	const auto Graph = dynamic_cast<GraphEntity*>(GetMutEntity(Provider, Id));
	Graph->Selection = NewType;
	for (const auto VertexId : Graph->VerticesIds)
		Provider.ExecuteCommand<VertexCommands::SetSelectionType>(VertexId, NewType);
}) {}
