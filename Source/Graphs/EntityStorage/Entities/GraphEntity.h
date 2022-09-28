#pragma once

#include "EntityId.h"

struct GraphEntity {
	constexpr static EntitySignature Signature = GRAPH;

	TMap<uint32_t, EntityId> VerticesCustomIdToEntityId;

	// used for edges duplicates identification
	TSet<uint32_t> EdgesHashes;

	// Serializable data
	bool Colorful;
	TSet<EntityId> Vertices;
	TSet<EntityId> Edges;
};
