#pragma once

#include "EntityId.h"

struct GraphEntity {
	constexpr static EntitySignature Signature = GRAPH;

	// TODO: maybe handle oriented graphs
	const bool IsOriented = false;

	TMap<uint32_t, EntityId> VertexUserIdToEntityId;
	TSet<uint32_t> EdgeHashes;

	// Serializable data
	bool Colorful;
	TArray<EntityId> VerticesIds;
	TArray<EntityId> EdgesIds;
};
