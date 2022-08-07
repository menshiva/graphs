#pragma once

#include "Entity.h"

struct EdgeEntity final : Entity {
	explicit EdgeEntity(const class AGraphProvider &Provider);

	void SetActorColor(const FLinearColor &NewColor) const;

	EntityId GraphId = ENTITY_NONE;
	TStaticArray<EntityId, 2> VerticesIds;

	uint32_t DisplayId = 0;
};
