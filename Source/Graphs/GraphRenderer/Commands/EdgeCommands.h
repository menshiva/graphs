#pragma once

#include "../GraphsRenderer.h"
#include "ThirdParty/rapidjson/document.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace EdgeCommands {
	struct Create final : GraphsRendererCommand {
		Create(
			EntityId GraphId, EntityId *NewEdgeId,
			EntityId FromVertexId, EntityId ToVertexId
		);

		Create(
			EntityId GraphId, EntityId *NewEdgeId,
			uint32_t FromVertexUserId, uint32_t ToVertexUserId
		);
	private:
		static bool CreateImpl(
			AGraphsRenderer &Renderer,
			EntityId GraphId, EntityId *NewEdgeId,
			EntityId FromVertexId, EntityId ToVertexId
		);
	};

	struct Remove final : GraphsRendererCommand {
		explicit Remove(EntityId EdgeId);
	};

	struct Reserve final : GraphsRendererCommand {
		Reserve(EntityId GraphId, uint32_t NewEdgesNum);
	};

	struct SetHit final : GraphsRendererCommand {
		SetHit(EntityId EdgeId, bool IsHit, bool MarkDirty = true);
	};

	struct SetOverrideColor final : GraphsRendererCommand {
		SetOverrideColor(EntityId EdgeId, const FColor &OverrideColor, bool MarkDirty = true);
	};

	struct Move final : GraphsRendererCommand {
		Move(EntityId EdgeId, const FVector &Delta);
	};

	namespace ConstFuncs {
		void Serialize(EntityId EdgeId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);

		bool Deserialize(EntityId GraphId, const rapidjson::Value &DomEdge, FString &ErrorMessage, EdgeEntity &NewEdge);

		uint32_t ComputeHash(const EdgeEntity &Edge, bool ReverseVerticesIds);
	}
}
