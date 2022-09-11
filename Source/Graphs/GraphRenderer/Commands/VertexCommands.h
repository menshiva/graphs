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

	struct Remove final : AGraphRenderer::Command {
		explicit Remove(const EntityId &VertexId);
	};

	struct SetSelection final : AGraphRenderer::Command {
		SetSelection(const EntityId &VertexId, EntitySelection NewSelection);
	};

	struct Move final : AGraphRenderer::Command {
		Move(const EntityId &VertexId, const FVector &Delta);
	};
}
