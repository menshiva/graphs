#pragma once

#include "EntityId.h"

struct GraphEntity {
	constexpr static EntitySignature Signature = EntitySignature::GRAPH;

	TMap<uint32_t, EntityId> VertexUserIdToEntityId;

	// Serializable data
	TArray<EntityId> VerticesIds;
	TArray<EntityId> EdgesIds;
};
