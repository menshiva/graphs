#pragma once

#include "../GraphRenderer.h"

namespace GraphCommands {
	struct Create final : AGraphRenderer::Command {
		explicit Create(EntityId *NewGraphId);
	};

	struct Remove final : AGraphRenderer::Command {
		explicit Remove(const EntityId &GraphId);
	};

	struct SetHit final : AGraphRenderer::Command {
		SetHit(const EntityId &GraphId, bool IsHit);
	};

	struct SetOverrideColor final : AGraphRenderer::Command {
		SetOverrideColor(const EntityId &GraphId, const FLinearColor &OverrideColor);
	};

	struct Move final : AGraphRenderer::Command {
		Move(const EntityId &GraphId, const FVector &Delta);
	};

	struct Rotate final : AGraphRenderer::Command {
		Rotate(const EntityId &GraphId, const FVector &Origin, float Angle);
	};

	namespace ConstFuncs {
		FVector ComputeCenterPosition(const EntityStorage &Storage, const EntityId &GraphId);
	}
}
