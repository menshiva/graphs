#pragma once

#include "EntityId.h"

/**
 * Represents an edge in a graph.
 *
 * An edge is a connection between two vertices, and has a weight and a hit flag.
 * It may also have an override color, which is used to override the edge's visual appearance.
 */
struct EdgeEntity {
	constexpr static EntitySignature Signature = EDGE;

	/** The ID of the graph to which this edge belongs. */
	EntityId GraphId = EntityId::NONE();

	/** The hit flag of this edge. Defines if this entity was hit by the right controller. */
	bool IsHit;

	/**
	 * The override color of this edge.
	 *
	 * If not set to the special value `ColorConsts::OverrideColorNone`, this color will be used to override the edge's
	 * default visual appearance.
	 */
	FColor OverrideColor;

	// Serializable data

	/** The IDs of the vertices connected by this edge. */
	TStaticArray<EntityId, 2> ConnectedVertices = TStaticArray<EntityId, 2>(EntityId::NONE());
	float Weight;
};
