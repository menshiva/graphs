#pragma once

#include "../EntityStorage.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace GraphCommands {
	namespace Mutable {
		EntityId Create(bool Colorful);

		void Remove(EntityId GraphId);

		void SetHit(EntityId GraphId, bool IsHit);

		void SetOverrideColor(EntityId GraphId, const FColor &OverrideColor);

		void Move(EntityId GraphId, const FVector &Delta);

		void Rotate(EntityId GraphId, const FVector &Origin, float Angle);

		void CompactSets(EntityId GraphId);

		EntityId Deserialize(const FString &JsonStr, FString &ErrorMessage);
	}

	namespace Const {
		void Serialize(EntityId GraphId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);

		FVector ComputeCenterPosition(EntityId GraphId);
	}
}
