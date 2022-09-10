#pragma once

#include "EntityId.h"

struct EdgeEntity {
	enum class SelectionType : uint8_t {
		NONE = 0,
		HIT,
		SELECTED
	};

	constexpr static EntitySignature Signature = EntitySignature::EDGE;

	EntityId GraphId;
	SelectionType Selection;

	// Serializable data
	TStaticArray<EntityId, 2> VerticesIds;
};
