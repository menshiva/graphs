#pragma once

#include "EntityId.h"
#include "EntitySelection.h"

struct EdgeEntity {
	constexpr static EntitySignature Signature = EntitySignature::EDGE;

	EntityId GraphId;
	EntitySelection Selection;

	// Serializable data
	TStaticArray<EntityId, 2> VerticesIds;
};
