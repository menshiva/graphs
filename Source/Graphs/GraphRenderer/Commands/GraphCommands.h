#pragma once

#include "../GraphRenderer.h"

namespace GraphCommands {
	struct Create final : AGraphRenderer::Command {
		explicit Create(EntityId *NewGraphId);
	};

	struct SetSelection final : AGraphRenderer::Command {
		SetSelection(const EntityId &GraphId, EntitySelection NewSelection);
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
