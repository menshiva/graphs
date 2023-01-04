#pragma once

#include "EntityId.h"

/**
 * Represents a vertex in a graph.
 *
 * A vertex is a point in the graph that is connected to other vertices by edges. It has a label, a position, a color,
 * and a hit flag. It may also have an override color, which is used to override the vertex's visual appearance
 * and preserve the color defined by the user.
 */
struct VertexEntity {
	constexpr static EntitySignature Signature = VERTEX;

	/** The ID of the graph to which this vertex belongs. */
	EntityId GraphId = EntityId::NONE();

	/** The hit flag of this vertex. Defines if this entity was hit by the right controller. */
	bool IsHit;

	/**
	 * The override color of this vertex.
	 *
	 * If not set to the special value `ColorConsts::OverrideColorNone`, this color will be used to override the
	 * vertex's visual appearance.
	 */
	FColor OverrideColor;

	/** The set of IDs of the edges connected to this vertex. */
	TSet<EntityId> ConnectedEdges;

	// Serializable data

	uint32_t Label;
	FVector Position;
	FColor Color;
};
