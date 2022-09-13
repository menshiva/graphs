#pragma once

#include "../GraphRenderer.h"
#include "ThirdParty/rapidjson/document.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace EdgeCommands {
	struct Create final : AGraphRenderer::Command {
		Create(
			const EntityId &GraphId, EntityId *NewEdgeId,
			const EntityId &FromVertexId, const EntityId &ToVertexId
		);

		Create(
			const EntityId &GraphId, EntityId *NewEdgeId,
			uint32_t FromVertexUserId, uint32_t ToVertexUserId
		);
	private:
		static bool CreateImpl(
			EntityStorage &Storage,
			const EntityId &GraphId, EntityId *NewEdgeId,
			const EntityId &FromVertexId, const EntityId &ToVertexId
		);
	};

	struct Remove final : AGraphRenderer::Command {
		explicit Remove(const EntityId &EdgeId);
	};

	struct Reserve final : AGraphRenderer::Command {
		Reserve(const EntityId &GraphId, uint32_t NewEdgesNum);
	};

	struct SetHit final : AGraphRenderer::Command {
		SetHit(const EntityId &EdgeId, bool IsHit);
	};

	struct SetOverrideColor final : AGraphRenderer::Command {
		SetOverrideColor(const EntityId &EdgeId, const FColor &OverrideColor);
	};

	struct Move final : AGraphRenderer::Command {
		Move(const EntityId &EdgeId, const FVector &Delta);
	};

	namespace ConstFuncs {
		void Serialize(
			const EntityStorage &Storage,
			const EntityId &EdgeId,
			rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
		);

		bool Deserialize(
			const EntityStorage &Storage,
			const EntityId &GraphId,
			const rapidjson::Value &DomEdge,
			FString &ErrorMessage,
			EdgeEntity &NewEdge
		);

		uint32_t ComputeHash(const EdgeEntity &Edge, bool ReverseVerticesIds);
	}
}
