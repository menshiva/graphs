#pragma once

#include "../EntityStorage.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace GraphCommands {
	namespace Mutable {
		EntityId Create(bool Colorful);

		void Remove(EntityId GraphId);

		FORCEINLINE void SetColorful(const EntityId GraphId, const bool Colorful) {
			ES::GetEntityMut<GraphEntity>(GraphId).Colorful = Colorful;
		}

		void SetHit(EntityId GraphId, bool IsHit);

		void SetColor(EntityId GraphId, const FColor &Color);

		void SetColor(EntityId GraphId, const TArray<FColor> &Colors);

		void RandomizeVerticesColors(EntityId GraphId);

		void SetOverrideColor(EntityId GraphId, const FColor &OverrideColor);

		void Move(EntityId GraphId, const FVector &Delta);

		void Rotate(EntityId GraphId, const FVector &Origin, const FVector &Axis, float Angle);

		EntityId Deserialize(const FString &JsonStr, FString &ErrorMessage);
	}

	namespace Const {
		void Serialize(EntityId GraphId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);

		FVector ComputeCenterPosition(EntityId GraphId);

		uint32_t GenerateUniqueVertexUserId(EntityId GraphId);
	}
}
