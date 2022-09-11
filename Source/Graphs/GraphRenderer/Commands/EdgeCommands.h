#pragma once

#include "../GraphRenderer.h"

namespace EdgeCommands {
	struct Create final : AGraphRenderer::Command {
		Create(
			const EntityId *GraphId, EntityId *NewEdgeId,
			uint32_t FromVertexUserId, uint32_t ToVertexUserId
		);
	};

	struct SetSelection final : AGraphRenderer::Command {
		SetSelection(const EntityId &EdgeId, EntitySelection NewSelection);
	};
}
