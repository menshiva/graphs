#pragma once

#include "../EntityStorage.h"
#include "ThirdParty/rapidjson/document.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace VertexCommands {
	namespace Mutable {
		/**
 		 * Creates a new vertex entity and add it to the specified graph.
 		 * 
 		 * @param GraphId The ID of the graph to add the vertex to.
 		 * @param Label The label to assign to the vertex.
 		 * @param Position The position of the vertex.
 		 * @param Color The color of the vertex.
 		 * @return The ID of the newly created vertex entity.
		 */
		EntityId Create(
			EntityId GraphId,
			uint32_t Label,
			const FVector &Position,
			const FColor &Color
		);

		/**
		 * Removes the vertex with the given ID from the graph it belongs to and deletes it.
		 *
		 * It also removes all connected edge entities.
		 * If the graph contains only one vertex, the graph itself will be removed too.
		 * 
		 * @param VertexId The ID of the vertex to be removed.
		 */
		void Remove(EntityId VertexId);

		FORCEINLINE void SetHit(const EntityId VertexId, const bool IsHit) {
			ES::GetEntityMut<VertexEntity>(VertexId).IsHit = IsHit;
		}

		FORCEINLINE void SetColor(const EntityId VertexId, const FColor &Color) {
			ES::GetEntityMut<VertexEntity>(VertexId).Color = Color;
		}

		FORCEINLINE void SetOverrideColor(const EntityId VertexId, const FColor &OverrideColor) {
			ES::GetEntityMut<VertexEntity>(VertexId).OverrideColor = OverrideColor;
		}

		/**
		 * Moves the vertex with the given ID by the given delta.
		 * 
		 * @param VertexId The ID of the vertex to be moved.
		 * @param Delta The displacement vector by which the vertex should be moved.
		 */
		FORCEINLINE void Move(const EntityId VertexId, const FVector &Delta) {
			ES::GetEntityMut<VertexEntity>(VertexId).Position += Delta;
		}

		/**
		 * Deserializes a vertex from a JSON object and creates it in the given graph.
		 *
		 * The JSON object should contain the "label" and "position" attributes, and may contain the "color" attribute.
		 *
		 * @param DomVertex The JSON object to be deserialized.
		 * @param GraphId The ID of the graph to which the new vertex should be added.
		 * @param[out] ErrorMessage Output string that will contain an error message in case of failure.
		 * @return True on success, false on failure.
		 */
		bool Deserialize(const rapidjson::Value &DomVertex, EntityId GraphId, FString &ErrorMessage);
	}

	namespace Const {
		/**
		 * Serializes the vertex with the given ID to a JSON object.
		 *
		 * The resulting JSON object will have the "label" and "position" attributes, and may have the "color"
		 * attribute if the vertex color was previously defined by the user.
		 *
		 * @param VertexId The ID of the vertex to be serialized.
		 * @param Writer The RapidJSON writer to which the serialized vertex should be written.
		 */
		void Serialize(EntityId VertexId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);

		/**
		 * Checks whether two vertices are connected by an edge.
		 * 
		 * @param FirstVertexId The ID of the first vertex.
		 * @param SecondVertexId The ID of the second vertex.
		 * @return True if the vertices are connected, false otherwise.
		 */
		bool AreConnected(EntityId FirstVertexId, EntityId SecondVertexId);
	}
}
