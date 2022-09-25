#pragma once

#include "EntityId.h"

struct EdgeEntity {
	constexpr static EntitySignature Signature = EDGE;

	EntityId GraphId;
	bool IsHit;
	FColor OverrideColor;

	// Serializable data
	TStaticArray<EntityId, 2> ConnectedVertices;
};

struct EdgeImportData {
	TStaticArray<uint32_t, 2> ConnectedVertexCustomIds;
};
