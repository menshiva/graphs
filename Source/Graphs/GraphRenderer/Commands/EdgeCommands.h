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

	struct SetHit final : AGraphRenderer::Command {
		SetHit(const EntityId &EdgeId, bool IsHit);
	};

	struct SetOverrideColor final : AGraphRenderer::Command {
		SetOverrideColor(const EntityId &EdgeId, const FLinearColor &OverrideColor);
	};

	struct Move final : AGraphRenderer::Command {
		Move(const EntityId &EdgeId, const FVector &Delta);
	};
}
