#pragma once

#include "../GraphsRenderer.h"
#include "ThirdParty/rapidjson/document.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace VertexCommands {
	struct Create final : GraphsRendererCommand {
		Create(
			EntityId GraphId, EntityId *NewVertexId,
			uint32_t UserId,
			const FVector &Position,
			const FColor &Color
		);
	};

	struct Remove final : GraphsRendererCommand {
		explicit Remove(EntityId VertexId);
	};

	struct Reserve final : GraphsRendererCommand {
		Reserve(EntityId GraphId, uint32_t NewVerticesNum);
	};

	struct SetHit final : GraphsRendererCommand {
		SetHit(EntityId VertexId, bool IsHit);
	};

	struct SetOverrideColor final : GraphsRendererCommand {
		SetOverrideColor(EntityId VertexId, const FColor &OverrideColor);
	};

	struct Move final : GraphsRendererCommand {
		Move(EntityId VertexId, const FVector &Delta);
	};

	namespace ConstFuncs {
		void Serialize(EntityId VertexId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);

		bool Deserialize(
			EntityId GraphId,
			const rapidjson::Value &DomVertex,
			FString &ErrorMessage,
			VertexEntity &NewVertex
		);
	}
}
