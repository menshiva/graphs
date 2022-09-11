#pragma once

#include "EntityId.h"
#include "EntitySelection.h"

struct VertexEntity {
	constexpr static EntitySignature Signature = EntitySignature::VERTEX;

	EntityId GraphId;
	EntitySelection Selection;
	TArray<EntityId> EdgesIds;

	// Serializable data
	uint32_t UserId;
	FVector Position;
	FLinearColor Color;
};
