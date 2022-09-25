#pragma once

#include "EdgeEntity.h"
#include "EntityId.h"
#include "VertexEntity.h"

struct GraphEntity {
	constexpr static EntitySignature Signature = GRAPH;

	TMap<uint32_t, EntityId> VerticesCustomIdToEntityId;

	// used for edges duplicates identification
	TSet<uint32_t> EdgesHashes;

	// Serializable data
	bool Colorful;
	TArray<EntityId> Vertices;
	TArray<EntityId> Edges;
};

struct GraphImportData {
	bool Colorful;
	TArray<VertexImportData> Vertices;
	TArray<EdgeImportData> Edges;
};
