#pragma once

#include "EntityId.h"

struct VertexEntity {
	constexpr static EntitySignature Signature = VERTEX;

	EntityId GraphId;
	bool IsHit;
	FColor OverrideColor;
	TSet<EntityId> ConnectedEdges;

	// Serializable data
	uint32_t CustomId;
	FVector Position;
	FColor Color;
	double Value;
};
