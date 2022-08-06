#include "GraphCommands.h"
#include "VertexCommands.h"
#include "Graphs/GraphProvider/Entities/GraphEntity.h"
#include "Graphs/GraphProvider/Entities/VertexEntity.h"

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

GraphCommands::Move::Move(
	EntityId Id,
	const FVector &Delta
) : Command([Id, &Delta] (AGraphProvider &Provider) {
	const auto Graph = dynamic_cast<const GraphEntity*>(GetConstEntity(Provider, Id));
	for (const auto VertexId : Graph->VerticesIds)
		Provider.ExecuteCommand<VertexCommands::Move>(VertexId, Delta);
}) {}

GraphCommands::ComputeCenterPosition::ComputeCenterPosition(
	EntityId Id,
	FVector &Center
) : Command([Id, &Center] (const AGraphProvider &Provider) {
	const auto Graph = dynamic_cast<const GraphEntity*>(GetConstEntity(Provider, Id));
	check(Graph->VerticesIds.Num() > 0);
	Center = FVector::ZeroVector;
	for (const auto VertexId : Graph->VerticesIds)
		Center += GetMutEntity(Provider, VertexId)->GetActor()->GetActorLocation();
	Center /= Graph->VerticesIds.Num();
}) {}

GraphCommands::Rotate::Rotate(
	EntityId Id,
	const FVector &Center,
	const float Angle
) : Command([Id, &Center, Angle] (AGraphProvider &Provider) {
	const auto Graph = dynamic_cast<const GraphEntity*>(GetConstEntity(Provider, Id));
	for (const auto VertexId : Graph->VerticesIds) {
		const auto Vertex = dynamic_cast<VertexEntity*>(GetMutEntity(Provider, VertexId));
		const auto VertexPos = Vertex->GetActor()->GetActorLocation();
		const auto PosDirection = VertexPos - Center;
		const auto RotatedPos = PosDirection.RotateAngleAxis(Angle, FVector::DownVector) + Center;
		Provider.ExecuteCommand<VertexCommands::Move>(VertexId, RotatedPos - VertexPos);
	}
}) {}
