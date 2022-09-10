#pragma once

#include "../GraphRenderer.h"

namespace GraphCommands {
	struct Create final : AGraphRenderer::Command {
		explicit Create(EntityId *NewGraphId);
	};
}
