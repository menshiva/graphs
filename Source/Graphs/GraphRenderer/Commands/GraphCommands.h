#pragma once

#include "../GraphRenderer.h"

namespace GraphCommands {
	struct Create final : AGraphRenderer::Command {
		explicit Create(EntityId *NewGraphId);
	};

	struct SetSelection final : AGraphRenderer::Command {
		SetSelection(const EntityId &GraphId, EntitySelection NewSelection);
	};
}
