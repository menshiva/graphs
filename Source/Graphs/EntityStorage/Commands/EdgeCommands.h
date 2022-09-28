#pragma once

#include "../EntityStorage.h"
#include "ThirdParty/rapidjson/document.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace EdgeCommands {
	namespace Mutable {
		EntityId Create(EntityId GraphId, EntityId FromVertexId, EntityId ToVertexId);

		void Remove(EntityId EdgeId);

		FORCEINLINE void SetHit(const EntityId EdgeId, const bool IsHit) {
			ES::GetEntityMut<EdgeEntity>(EdgeId).IsHit = IsHit;
		}

		FORCEINLINE void SetOverrideColor(const EntityId EdgeId, const FColor &OverrideColor) {
			ES::GetEntityMut<EdgeEntity>(EdgeId).OverrideColor = OverrideColor;
		}

		void Move(EntityId EdgeId, const FVector &Delta);

		bool Deserialize(
			const rapidjson::Value &DomEdge,
			EntityId GraphId,
			FString &ErrorMessage
		);
	}

	namespace Const {
		void Serialize(EntityId EdgeId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);

		uint32_t ComputeHash(const EdgeEntity &Edge, bool ReverseVerticesIds);
	}
}
