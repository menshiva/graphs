#pragma once

#include "Graphs/GraphProvider/GraphProvider.h"
#include "ThirdParty/rapidjson/prettywriter.h"
#include "ThirdParty/rapidjson/stringbuffer.h"
#include "ThirdParty/rapidjson/document.h"

namespace EdgeCommands {
	struct Create final : AGraphProvider::Command {
		Create(
			EntityId GraphId,
			EntityId *NewEdgeId,
			EntityId FromVertexId, EntityId ToVertexId
		);
	};

	struct Remove final : AGraphProvider::Command {
		explicit Remove(EntityId Id);
	};

	struct GetGraphId final : AGraphProvider::Command {
		GetGraphId(EntityId Id, EntityId &GraphId);
	};

	struct UpdateTransform final : AGraphProvider::Command {
		explicit UpdateTransform(EntityId Id);
	};

	struct SetColor final : AGraphProvider::Command {
		SetColor(EntityId Id, const FLinearColor &Color);
	};

	struct SetSelectionType final : AGraphProvider::Command {
		SetSelectionType(EntityId Id, SelectionType NewType);
	};

	struct Move final : AGraphProvider::Command {
		Move(EntityId Id, const FVector &Delta, bool UpdateConnectedVertices);
	};

	struct Serialize final : AGraphProvider::Command {
		Serialize(EntityId Id, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);
	};

	struct Deserialize final : AGraphProvider::Command {
		Deserialize(
			EntityId GraphId,
			EntityId *NewEdgeId,
			rapidjson::Value &EdgeDomValue,
			const TMap<uint32_t, EntityId> &VerticesIdsMapping,
			FString &ErrorMessage
		);
	};
}
