#include "GraphCommands.h"
#include "VertexCommands.h"
#include "EdgeCommands.h"

GraphCommands::Create::Create(EntityId *NewGraphId) : Command([=] (EntityStorage &Storage) -> bool {
	const auto GraphId = Storage.NewEntity<GraphEntity>();
	if (NewGraphId)
		*NewGraphId = GraphId;
	return true;
}) {}

GraphCommands::Remove::Remove(const EntityId &GraphId) : Command([=] (EntityStorage &Storage) -> bool {
	const auto &Graph = Storage.GetEntity<GraphEntity>(GraphId);

	for (const auto &VertexId : Graph.VerticesIds)
		Storage.RemoveEntity<VertexEntity>(VertexId);
	for (const auto &EdgeId : Graph.EdgesIds)
		Storage.RemoveEntity<EdgeEntity>(EdgeId);
	Storage.RemoveEntity<GraphEntity>(GraphId);

	return true;
}) {}

GraphCommands::SetSelection::SetSelection(
	const EntityId &GraphId,
	const EntitySelection NewSelection
) : Command([=] (EntityStorage &Storage) -> bool {
	const auto &Graph = Storage.GetEntity<GraphEntity>(GraphId);
	bool ChangesProvided = false;

	for (const auto &VertexId : Graph.VerticesIds) {
		const bool IsSuccess = ExecuteSubCommand(
			VertexCommands::SetSelection(VertexId, NewSelection),
			Storage
		);
		if (!ChangesProvided)
			ChangesProvided = IsSuccess;
	}

	for (const auto &EdgeId : Graph.EdgesIds) {
		const bool IsSuccess = ExecuteSubCommand(
			EdgeCommands::SetSelection(EdgeId, NewSelection),
			Storage
		);
		if (!ChangesProvided)
			ChangesProvided = IsSuccess;
	}

	return ChangesProvided;
}) {}

GraphCommands::Move::Move(
	const EntityId &GraphId,
	const FVector &Delta
) : Command([=] (EntityStorage &Storage) -> bool {
	const auto &Graph = Storage.GetEntity<GraphEntity>(GraphId);

	for (const auto &VertexId : Graph.VerticesIds)
		ExecuteSubCommand(VertexCommands::Move(VertexId, Delta), Storage);

	return true;
}) {}

GraphCommands::Rotate::Rotate(
	const EntityId &GraphId,
	const FVector &Origin,
	const float Angle
) : Command([=] (EntityStorage &Storage) -> bool {
	const auto &Graph = Storage.GetEntity<GraphEntity>(GraphId);

	for (const auto &VertexId : Graph.VerticesIds) {
		const auto &Vertex = Storage.GetEntity<VertexEntity>(VertexId);
		const auto PosDir = Vertex.Position - Origin;
		const auto RotatedPos = PosDir.RotateAngleAxis(Angle, FVector::DownVector) + Origin;
		ExecuteSubCommand(VertexCommands::Move(VertexId, RotatedPos - Vertex.Position), Storage);
	}

	return true;
}) {}

FVector GraphCommands::ConstFuncs::ComputeCenterPosition(const EntityStorage &Storage, const EntityId &GraphId) {
	const auto &Graph = Storage.GetEntity<GraphEntity>(GraphId);
	check(Graph.VerticesIds.Num() > 0);

	FVector Center = FVector::ZeroVector;
	for (const auto &VertexId : Graph.VerticesIds)
		Center += Storage.GetEntity<VertexEntity>(VertexId).Position;
	Center /= Graph.VerticesIds.Num();

	return Center;
}
