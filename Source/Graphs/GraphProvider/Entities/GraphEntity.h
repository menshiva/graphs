#pragma once

#include "Entity.h"

struct GraphEntity final : Entity {
	explicit GraphEntity(const class AGraphProvider &Provider);

	TArray<EntityId> VerticesIds;
	TArray<EntityId> EdgesIds;
};
