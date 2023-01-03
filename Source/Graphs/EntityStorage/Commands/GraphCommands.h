#pragma once

#include "../EntityStorage.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace GraphCommands {
	namespace Mutable {
		/**
		  * Creates a new empty graph entity.
		  * @param Colorful Whether the graph should be colorful.
		  * @return The ID of the newly created graph entity.
		 */
		EntityId Create(bool Colorful);

		/**
		 * Removes a graph entity, including all its vertices and edges.
		 * @param GraphId The ID of the graph entity to remove.
		 */
		void Remove(EntityId GraphId);

		FORCEINLINE void SetColorful(const EntityId GraphId, const bool Colorful) {
			ES::GetEntityMut<GraphEntity>(GraphId).Colorful = Colorful;
		}

		/**
		 * Sets the hit flag for all vertices and edges in the specified graph.
		 * @param GraphId The ID of the graph.
		 * @param IsHit The new hit flag.
		 */
		void SetHit(EntityId GraphId, bool IsHit);

		/**
		 * Sets the color of all vertices in the graph to the specified value.
		 * @param GraphId The ID of the graph to set the color of.
		 * @param Color Value to set the color to.
		 */
		void SetColor(EntityId GraphId, const FColor &Color);

		/**
		 * Sets the color of all vertices in the graph to the given array of colors.
		 * The number of colors must be equal to the number of vertices in the graph.
		 * @param GraphId Entity ID of the graph to set the color of.
		 * @param Colors Array of colors to set the vertices to.
		 */
		void SetColor(EntityId GraphId, const TArray<FColor> &Colors);

		/**
		 * Sets the color of each vertex in the specified graph to a random color.
		 * @param GraphId The ID of the graph whose vertex colors to randomize.
		 */
		void RandomizeVerticesColors(EntityId GraphId);

		/**
		 * Sets the override color of all vertices and edges in the given graph.
		 * @param GraphId The entity ID of the graph.
		 * @param OverrideColor The color to set.
		 */
		void SetOverrideColor(EntityId GraphId, const FColor &OverrideColor);

		/**
		 * Moves all vertices in the graph by a given delta.
		 * @param GraphId The ID of the graph.
		 * @param Delta The displacement vector by which each vertex should be moved.
		 */
		void Move(EntityId GraphId, const FVector &Delta);

		/**
		 * Rotates all the vertices of the given graph around the specified origin by the given angle.
		 * @param GraphId The ID of the graph to rotate.
		 * @param Origin The origin around which the rotation will happen.
		 * @param Axis The axis of rotation.
		 * @param Angle The angle of rotation, in degrees.
		 */
		void Rotate(EntityId GraphId, const FVector &Origin, const FVector &Axis, float Angle);

		/**
		 * Deserializes a JSON string and creates a graph with it.
		 * @param JsonStr The JSON string to be deserialized.
		 * @param[out] ErrorMessage If the function returns EntityId::NONE, this will contain the error message.
		 * @return The ID of the created graph, or EntityId::NONE if failed.
		 */
		EntityId Deserialize(const FString &JsonStr, FString &ErrorMessage);
	}

	namespace Const {
		/**
		 * Serializes the given graph entity.
		 * @param GraphId The ID of the graph entity to serialize.
		 * @param Writer Output buffer for the serialized data.
		 */
		void Serialize(EntityId GraphId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);

		/**
		 * Calculates the center position of the graph by averaging the positions of all its vertices.
		 * @param GraphId The ID of the graph to compute the center position for.
		 * @return The center position of the graph.
		 */
		FVector ComputeCenterPosition(EntityId GraphId);

		/**
		 * Generates a unique vertex label for the specified graph.
		 *
		 * This function looks for the highest vertex label in the graph and returns it + 1.
		 * This ensures that the returned label is unique in the graph.
		 *
		 * @param GraphId The ID of the graph.
		 * @return The unique vertex label.
		 */
		uint32_t GenerateUniqueVertexLabel(EntityId GraphId);
	}
}
