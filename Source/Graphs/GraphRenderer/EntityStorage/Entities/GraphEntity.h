#pragma once

#include "EntityId.h"

struct GraphEntity {
	constexpr static EntitySignature Signature = EntitySignature::GRAPH;

	TMap<uint32_t, EntityId> VertexUserIdToEntityId;
	TSet<uint32_t> EdgeHashes;

	// Serializable data
	bool UseDefaultColors;
	TArray<EntityId> VerticesIds;
	TArray<EntityId> EdgesIds;
};
