#pragma once

#include "../GraphRenderer.h"

namespace EdgeCommands {
	struct Create final : AGraphRenderer::Command {
		Create(
			const EntityId *GraphId, EntityId *NewEdgeId,
			uint32_t FromVertexUserId, uint32_t ToVertexUserId,
			const FLinearColor &Color
		);
	};
}
