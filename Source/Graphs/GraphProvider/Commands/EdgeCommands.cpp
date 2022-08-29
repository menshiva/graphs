#include "EdgeCommands.h"
#include "VertexCommands.h"
#include "Graphs/GraphProvider/Entities/EdgeEntity.h"
#include "Graphs/GraphProvider/Entities/GraphEntity.h"
#include "Graphs/GraphProvider/Entities/VertexEntity.h"
#include "Graphs/Utils/Consts.h"

EdgeCommands::Create::Create(
	EntityId GraphId,
	EntityId FirstVertexId, EntityId SecondVertexId,
	EntityId *NewEdgeId
) : Command([GraphId, FirstVertexId, SecondVertexId, NewEdgeId] (AGraphProvider &Provider) {
	const auto NewEdge = CreateEntity<EdgeEntity>(Provider);

	const auto Graph = GetEntity<GraphEntity>(Provider, GraphId);
	NewEdge->Actor->AttachToActor(Graph->Actor.Get(), FAttachmentTransformRules::KeepWorldTransform);
	check(!Graph->EdgesIds.Contains(NewEdge->GetEntityId()));
	Graph->EdgesIds.Push(NewEdge->GetEntityId());

	NewEdge->GraphId = GraphId;
	NewEdge->VerticesIds[0] = FirstVertexId;
	NewEdge->VerticesIds[1] = SecondVertexId;

	for (const auto VertexId : NewEdge->VerticesIds) {
		const auto Vertex = GetEntity<VertexEntity>(Provider, VertexId);
		check(!Vertex->EdgesIds.Contains(NewEdge->GetEntityId()));
		Vertex->EdgesIds.Push(NewEdge->GetEntityId());
	}

	Provider.ExecuteCommand(UpdateTransform(NewEdge->GetEntityId()));

	if (NewEdgeId)
		*NewEdgeId = NewEdge->GetEntityId();
}) {}

EdgeCommands::Remove::Remove(EntityId Id) : Command([Id] (AGraphProvider &Provider) {
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

EdgeCommands::GetGraphId::GetGraphId(
	EntityId Id,
	EntityId &GraphId
) : Command([Id, &GraphId] (const AGraphProvider &Provider) {
	GraphId = GetEntity<const EdgeEntity>(Provider, Id)->GraphId;
}) {}

EdgeCommands::UpdateTransform::UpdateTransform(EntityId Id) : Command([Id] (const AGraphProvider &Provider) {
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

EdgeCommands::SetColor::SetColor(
	EntityId Id,
	const FLinearColor &Color
) : Command([Id, &Color] (const AGraphProvider &Provider) {
	const auto Edge = GetEntity<EdgeEntity>(Provider, Id);
	Edge->SetActorColor(Color);
}) {}

EdgeCommands::SetSelectionType::SetSelectionType(
	EntityId Id,
	SelectionType NewType
) : Command([Id, NewType] (AGraphProvider &Provider) {
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

EdgeCommands::Move::Move(
	EntityId Id,
	const FVector& Delta,
	bool UpdateConnectedVertices
) : Command([Id, &Delta, UpdateConnectedVertices] (AGraphProvider &Provider) {
	const auto Edge = GetEntity<const EdgeEntity>(Provider, Id);
	if (UpdateConnectedVertices) {
		for (const auto VertexId : Edge->VerticesIds)
			Provider.ExecuteCommand(VertexCommands::Move(VertexId, Delta, true));
	}
	else {
		Edge->Actor->SetActorLocation(Edge->Actor->GetActorLocation() + Delta);
	}
}) {}
