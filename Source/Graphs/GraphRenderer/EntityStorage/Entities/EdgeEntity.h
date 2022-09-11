#pragma once

#include "EntityId.h"

struct EdgeEntity {
	constexpr static EntitySignature Signature = EntitySignature::EDGE;

	EntityId GraphId;
	bool IsHit;
	FLinearColor OverrideColor;

	// Serializable data
	TStaticArray<EntityId, 2> VerticesIds;
};
