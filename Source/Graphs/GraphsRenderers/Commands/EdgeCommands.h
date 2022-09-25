#pragma once

#include "../GraphsRenderers.h"
#include "ThirdParty/rapidjson/document.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace EdgeCommands {
	struct Create final : GraphsRenderersCommand {
		Create(
			EntityId GraphId, EntityId *NewEdgeId,
			EntityId FromVertexId, EntityId ToVertexId
		);
	};

	struct Remove final : GraphsRenderersCommand {
		explicit Remove(EntityId EdgeId);
	};

	struct Reserve final : GraphsRenderersCommand {
		Reserve(EntityId GraphId, uint32_t NewEdgesNum);
	};

	struct SetHit final : GraphsRenderersCommand {
		SetHit(EntityId EdgeId, bool IsHit);
	};

	struct SetOverrideColor final : GraphsRenderersCommand {
		SetOverrideColor(EntityId EdgeId, const FColor &OverrideColor);
	};

	struct Move final : GraphsRenderersCommand {
		Move(EntityId EdgeId, const FVector &Delta);
	};

	namespace Consts {
		void Serialize(EntityId EdgeId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);

		bool Deserialize(
			const rapidjson::Value &DomEdge,
			GraphImportData &GraphData,
			const TSet<uint32_t> &VerticesCustomIds,
			TSet<uint32_t> &ImportEdgesHashes,
			FString &ErrorMessage
		);

		uint32_t ComputeHash(const EdgeEntity &Edge, bool ReverseVerticesIds);
	}
}
