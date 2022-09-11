#pragma once

#include "../GraphRenderer.h"

namespace VertexCommands {
	struct Create final : AGraphRenderer::Command {
		Create(
			const EntityId *GraphId, EntityId *NewVertexId,
			uint32_t UserId,
			const FVector &Position,
			const FLinearColor &Color
		);
	};

	struct SetSelection final : AGraphRenderer::Command {
		SetSelection(const EntityId &VertexId, EntitySelection NewSelection);
	};
}
