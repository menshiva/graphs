#pragma once

#include "../GraphsRenderers.h"
#include "ThirdParty/rapidjson/document.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace VertexCommands {
	struct Create final : GraphsRenderersCommand {
		Create(
			EntityId GraphId, EntityId *NewVertexId,
			uint32_t CustomVertexId,
			const FVector &Position,
			const FColor &Color
		);
	};

	struct Remove final : GraphsRenderersCommand {
		explicit Remove(EntityId VertexId);
	};

	struct Reserve final : GraphsRenderersCommand {
		Reserve(EntityId GraphId, uint32_t NewVerticesNum);
	};

	struct SetHit final : GraphsRenderersCommand {
		SetHit(EntityId VertexId, bool IsHit);
	};

	struct SetOverrideColor final : GraphsRenderersCommand {
		SetOverrideColor(EntityId VertexId, const FColor &OverrideColor);
	};

	struct Move final : GraphsRenderersCommand {
		Move(EntityId VertexId, const FVector &Delta);
	};

	namespace Consts {
		void Serialize(EntityId VertexId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);

		bool Deserialize(
			const rapidjson::Value &DomVertex,
			GraphImportData &GraphData,
			TSet<uint32_t> &VerticesCustomIds,
			FString &ErrorMessage
		);
	}
}
