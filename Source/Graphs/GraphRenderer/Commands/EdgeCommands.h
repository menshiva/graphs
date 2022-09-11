#pragma once

#include "../GraphRenderer.h"

namespace EdgeCommands {
	struct Create final : AGraphRenderer::Command {
		Create(
			const EntityId *GraphId, EntityId *NewEdgeId,
			uint32_t FromVertexUserId, uint32_t ToVertexUserId
		);
	};

	struct Remove final : AGraphRenderer::Command {
		explicit Remove(const EntityId &EdgeId);
	};

	struct SetSelection final : AGraphRenderer::Command {
		SetSelection(const EntityId &EdgeId, EntitySelection NewSelection);
	};

	struct Move final : AGraphRenderer::Command {
		Move(const EntityId &EdgeId, const FVector &Delta);
	};
}
