#pragma once

#include "../GraphRenderer.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace GraphCommands {
	struct Create final : AGraphRenderer::Command {
		Create(EntityId *NewGraphId, bool UseDefaultColors);
	};

	struct Remove final : AGraphRenderer::Command {
		explicit Remove(const EntityId &GraphId);
	};

	struct SetHit final : AGraphRenderer::Command {
		SetHit(const EntityId &GraphId, bool IsHit);
	};

	struct SetOverrideColor final : AGraphRenderer::Command {
		SetOverrideColor(const EntityId &GraphId, const FColor &OverrideColor);
	};

	struct Move final : AGraphRenderer::Command {
		Move(const EntityId &GraphId, const FVector &Delta);
	};

	struct Rotate final : AGraphRenderer::Command {
		Rotate(const EntityId &GraphId, const FVector &Origin, float Angle);
	};

	struct Deserialize final : AGraphRenderer::Command {
		Deserialize(const FString &JsonStr, FString &ErrorMessage);
	};

	namespace ConstFuncs {
		void Serialize(
			const EntityStorage &Storage,
			const EntityId &GraphId,
			rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
		);

		FVector ComputeCenterPosition(const EntityStorage &Storage, const EntityId &GraphId);

		bool IsSetContainsGraphChildrenEntities(
			const EntityStorage &Storage,
			const EntityId &GraphId,
			const TSet<EntityId> &InSet
		);
	}
}
