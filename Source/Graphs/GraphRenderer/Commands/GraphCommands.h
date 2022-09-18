#pragma once

#include "../GraphsRenderer.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace GraphCommands {
	struct Create final : GraphsRendererCommand {
		Create(EntityId *NewGraphId, bool Colorful);
	};

	struct Remove final : GraphsRendererCommand {
		explicit Remove(EntityId GraphId);
	};

	struct UpdateCollisions final : GraphsRendererCommand {
		UpdateCollisions();
	};

	struct SetHit final : GraphsRendererCommand {
		SetHit(EntityId GraphId, bool IsHit);
	};

	struct SetOverrideColor final : GraphsRendererCommand {
		SetOverrideColor(EntityId GraphId, const FColor &OverrideColor);
	};

	struct Move final : GraphsRendererCommand {
		Move(EntityId GraphId, const FVector &Delta);
	};

	struct Rotate final : GraphsRendererCommand {
		Rotate(EntityId GraphId, const FVector &Origin, float Angle);
	};

	struct Deserialize final : GraphsRendererCommand {
		Deserialize(const FString &JsonStr, FString &ErrorMessage);
	};

	namespace ConstFuncs {
		void Serialize(EntityId GraphId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);

		FVector ComputeCenterPosition(EntityId GraphId);

		bool IsSetContainsGraphChildrenEntities(EntityId GraphId, const TSet<EntityId> &InSet);
	}
}
