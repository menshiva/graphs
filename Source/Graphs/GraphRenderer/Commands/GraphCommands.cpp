#include "GraphCommands.h"

GraphCommands::Create::Create(EntityId *NewGraphId) : Command([=] (EntityStorage &Storage) {
	const auto GraphId = Storage.NewEntity<GraphEntity>();
	if (NewGraphId)
		*NewGraphId = GraphId;
}) {}
