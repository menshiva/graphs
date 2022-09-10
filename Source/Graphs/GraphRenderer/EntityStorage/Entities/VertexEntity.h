#pragma once

#include "EntityId.h"

struct VertexEntity {
	enum class SelectionType : uint8_t {
		NONE = 0,
		HIT,
		SELECTED
	};

	constexpr static EntitySignature Signature = EntitySignature::VERTEX;

	EntityId GraphId;
	SelectionType Selection;
	TArray<EntityId> EdgesIds;

	// Serializable data
	uint32_t UserId;
	FVector Position;
	FLinearColor Color;
};
