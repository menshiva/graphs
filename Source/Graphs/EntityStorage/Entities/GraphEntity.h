#pragma once

#include "EntityId.h"

/**
 * Represents a graph, which is a collection of vertices and edges.
 *
 * A graph may be colorful, meaning that its vertices and edges can have different colors. It is composed of a set of
 * vertices and a set of edges, and has a fast look-up map to find vertex entity IDs by their labels.
 */
struct GraphEntity {
	constexpr static EntitySignature Signature = GRAPH;

	/** A map used for a fast look-up of a vertex entity ID by its label. */
	TMap<uint32_t, EntityId> VerticesLabelToEntityId;

	/** A set used to detect duplicate edges. It stores the hashes of the edges in the graph. */
	TSet<uint32_t> EdgesHashes;

	// Serializable data

	/**
	 * Whether this graph is colorful.
	 *
	 * If true, the vertices and edges in this graph will be rendered with their own different colors.
	 * If false, they all will be rendered with the same `ColorConsts::VertexDefaultColor` color.
	 */
	bool Colorful;
	TSet<EntityId> Vertices;
	TSet<EntityId> Edges;
};
