#include "GraphCommands.h"
#include "VertexCommands.h"
#include "EdgeCommands.h"

GraphCommands::Create::Create(EntityId *NewGraphId) : Command([=] (EntityStorage &Storage) -> bool {
	const auto GraphId = Storage.NewEntity<GraphEntity>();
	if (NewGraphId)
		*NewGraphId = GraphId;
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
