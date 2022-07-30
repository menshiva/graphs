#pragma once

#include "Entity.h"

class GraphEntity final : public Entity {
public:
	explicit GraphEntity(const class AGraphProvider &Provider);

	TArray<EntityId> VerticesIds;
};
