#pragma once

#include "Entity.h"

struct VertexEntity final : Entity {
	explicit VertexEntity(const class AGraphProvider &Provider);

	void SetActorColor(const FLinearColor &NewColor) const;

	EntityId GraphId = ENTITY_NONE;
};
