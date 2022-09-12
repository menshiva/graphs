#pragma once

#include "EntityId.h"

struct VertexEntity {
	constexpr static EntitySignature Signature = EntitySignature::VERTEX;

	EntityId GraphId;
	bool IsHit;
	FLinearColor OverrideColor;
	TSet<EntityId> EdgesIds;

	// Serializable data
	uint32_t UserId;
	FVector Position;
	FLinearColor Color;
};
