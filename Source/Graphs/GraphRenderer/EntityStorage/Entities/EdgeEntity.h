#pragma once

#include "EntityId.h"

struct EdgeEntity {
	constexpr static EntitySignature Signature = EDGE;

	EntityId GraphId;
	bool IsHit;
	FColor OverrideColor;

	// Serializable data
	TStaticArray<EntityId, 2> VerticesIds;
};
