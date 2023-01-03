#pragma once

#include "../EntityStorage.h"
#include "ThirdParty/rapidjson/document.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace EdgeCommands {
	namespace Mutable {
		/**
		 * Creates a new edge entity connecting the two given vertex entities, and adds it to the given graph.
 		 * @param GraphId The ID of the graph to which the new edge should be added.
 		 * @param FromVertexId The ID of the vertex at one end of the new edge.
 		 * @param ToVertexId The ID of the vertex at the other end of the new edge.
		 * @param Weight The weight of the new edge. Default is 0.0.
 		 * @return The ID of the new edge.
		 */
		EntityId Create(EntityId GraphId, EntityId FromVertexId, EntityId ToVertexId, float Weight = 0.0f);

		/**
		 * Removes the edge with the given ID from its associated graph and connected vertices, and deletes it.
		 * @param EdgeId The ID of the edge to be removed.
		 */
		void Remove(EntityId EdgeId);


		FORCEINLINE void SetHit(const EntityId EdgeId, const bool IsHit) {
			ES::GetEntityMut<EdgeEntity>(EdgeId).IsHit = IsHit;
		}

		FORCEINLINE void SetOverrideColor(const EntityId EdgeId, const FColor &OverrideColor) {
			ES::GetEntityMut<EdgeEntity>(EdgeId).OverrideColor = OverrideColor;
		}

		FORCEINLINE void SetWeight(const EntityId EdgeId, const float Weight) {
			ES::GetEntityMut<EdgeEntity>(EdgeId).Weight = Weight;
		}

		/**
		 * Moves the edge with the given ID by moving its connected vertices by the given delta.
		 * @param EdgeId The ID of the edge to be moved.
		 * @param Delta The displacement vector by which the edge should be moved.
		 */
		void Move(EntityId EdgeId, const FVector &Delta);

		/**
		 * Deserializes an edge from a JSON object and creates it in the given graph.
		 *
		 * The JSON object should contain the "from" and "to" integer numbers, and may contain the "weight" floating-point number.
		 *
		 * @param DomEdge The JSON object to be deserialized.
 		 * @param GraphId The ID of the graph to which the new edge should be added.
 		 * @param[out] ErrorMessage Output string that will contain an error message in case of failure.
 		 * @return True on success, false on failure.
		 */
		bool Deserialize(const rapidjson::Value &DomEdge, EntityId GraphId, FString &ErrorMessage);
	}

	namespace Const {
		/**
		 * Serializes the edge with the given ID to a JSON object.
 		 *
 		 * The resulting JSON object will have the "from" and "to" integer numbers representing the labels of the
 		 * connected vertices, and may have the "weight" floating-point number if the edge has a non-zero weight.
 		 *
 		 * @param EdgeId The ID of the edge to be serialized.
 		 * @param Writer The RapidJSON writer to which the serialized edge should be written.
 		 */
		void Serialize(EntityId EdgeId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);

		/**
 		 * Computes a hash value for the given edge.
 		 *
 		 * @param Edge The edge for which to compute the hash.
 		 * @param ReverseVerticesIds If true, the hash value will be computed for the edge with the vertices IDs reversed.
 		 * @return The computed hash value.
 		 */
		uint32_t ComputeHash(const EdgeEntity &Edge, bool ReverseVerticesIds);
	}
}
