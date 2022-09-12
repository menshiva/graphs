﻿#pragma once

#include "../GraphRenderer.h"
#include "ThirdParty/rapidjson/document.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace VertexCommands {
	struct Create final : AGraphRenderer::Command {
		Create(
			const EntityId &GraphId, EntityId *NewVertexId,
			uint32_t UserId,
			const FVector &Position,
			const FLinearColor &Color
		);
	};

	struct Remove final : AGraphRenderer::Command {
		explicit Remove(const EntityId &VertexId);
	};

	struct Reserve final : AGraphRenderer::Command {
		Reserve(const EntityId &GraphId, uint32_t NewVerticesNum);
	};

	struct SetHit final : AGraphRenderer::Command {
		SetHit(const EntityId &VertexId, bool IsHit);
	};

	struct SetOverrideColor final : AGraphRenderer::Command {
		SetOverrideColor(const EntityId &VertexId, const FLinearColor &OverrideColor);
	};

	struct Move final : AGraphRenderer::Command {
		Move(const EntityId &VertexId, const FVector &Delta);
	};

	namespace ConstFuncs {
		void Serialize(
			const EntityStorage &Storage,
			const EntityId &VertexId,
			rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
		);

		bool Deserialize(
			const EntityStorage &Storage,
			const EntityId &GraphId,
			const rapidjson::Value &DomVertex,
			FString &ErrorMessage,
			VertexEntity &NewVertex
		);
	}
}
