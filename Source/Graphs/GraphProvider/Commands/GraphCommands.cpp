#include "GraphCommands.h"
#include "EdgeCommands.h"
#include "VertexCommands.h"
#include "Graphs/GraphProvider/Entities/GraphEntity.h"
#include "Graphs/GraphProvider/Entities/VertexEntity.h"
#include "Graphs/Utils/Consts.h"

GraphCommands::Create::Create(EntityId *NewId) : Command([NewId] (AGraphProvider &Provider) {
	const auto NewNode = CreateEntity<GraphEntity>(Provider);
	if (NewId)
		*NewId = NewNode->GetEntityId();
}) {}

GraphCommands::Remove::Remove(EntityId Id) : Command([Id] (AGraphProvider &Provider) {
	const auto Graph = GetEntity<GraphEntity>(Provider, Id);
	for (const auto VertexId : Graph->VerticesIds)
		RemoveEntity(Provider, VertexId);
	for (const auto EdgeId : Graph->EdgesIds)
		RemoveEntity(Provider, EdgeId);
	RemoveEntity(Provider, Id);
}) {}

GraphCommands::SetColor::SetColor(
	EntityId Id,
	const FLinearColor &Color
) : Command([Id, &Color] (AGraphProvider &Provider) {
	const auto Graph = GetEntity<GraphEntity>(Provider, Id);
	for (const auto VertexId : Graph->VerticesIds)
		Provider.ExecuteCommand(VertexCommands::SetColor(VertexId, Color));
	for (const auto EdgeId : Graph->EdgesIds)
		Provider.ExecuteCommand(EdgeCommands::SetColor(EdgeId, Color));
}) {}

GraphCommands::SetSelectionType::SetSelectionType(
	EntityId Id,
	SelectionType NewType
) : Command([Id, NewType] (AGraphProvider &Provider) {
	const auto Graph = GetEntity<GraphEntity>(Provider, Id);
	for (const auto VertexId : Graph->VerticesIds)
		Provider.ExecuteCommand(VertexCommands::SetSelectionType(VertexId, NewType));
	for (const auto EdgeId : Graph->EdgesIds)
		Provider.ExecuteCommand(EdgeCommands::SetSelectionType(EdgeId, NewType));
}) {}

GraphCommands::Move::Move(
	EntityId Id,
	const FVector &Delta
) : Command([Id, &Delta] (AGraphProvider &Provider) {
	const auto Graph = GetEntity<const GraphEntity>(Provider, Id);
	for (const auto VertexId : Graph->VerticesIds)
		Provider.ExecuteCommand(VertexCommands::Move(VertexId, Delta, false));
	for (const auto EdgeId : Graph->EdgesIds)
		Provider.ExecuteCommand(EdgeCommands::Move(EdgeId, Delta, false));
}) {}

GraphCommands::ComputeCenterPosition::ComputeCenterPosition(
	EntityId Id,
	FVector &Center
) : Command([Id, &Center] (const AGraphProvider &Provider) {
	const auto Graph = GetEntity<const GraphEntity>(Provider, Id);
	check(Graph->VerticesIds.Num() > 0);
	Center = FVector::ZeroVector;
	for (const auto VertexId : Graph->VerticesIds)
		Center += GetEntity<const VertexEntity>(Provider, VertexId)->Actor->GetActorLocation();
	Center /= Graph->VerticesIds.Num();
}) {}

GraphCommands::Rotate::Rotate(
	EntityId Id,
	const FVector &Center,
	const float Angle
) : Command([Id, &Center, Angle] (AGraphProvider &Provider) {
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

GraphCommands::Export::Export(
	EntityId Id,
	bool &Result,
	FString &ResultMessage
) : Command([Id, &Result, &ResultMessage] (const AGraphProvider &Provider) {
	const auto Graph = GetEntity<const GraphEntity>(Provider, Id);

	// TODO
	Result = true;
	ResultMessage = FileConsts::ImportExportDir + "Test.json";
}) {}
