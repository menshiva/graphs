#pragma once

#include "Graphs/GraphProvider/GraphProvider.h"
#include "ThirdParty/rapidjson/prettywriter.h"
#include "ThirdParty/rapidjson/reader.h"

namespace VertexCommands {
	struct Create final : AGraphProvider::Command {
		Create(EntityId GraphId, EntityId *NewVertexId, uint32_t VertexDisplayId, const FVector &Position);
	};

	struct Remove final : AGraphProvider::Command {
		explicit Remove(EntityId Id);
	};

	struct GetGraphId final : AGraphProvider::Command {
		GetGraphId(EntityId Id, EntityId &GraphId);
	};

	struct SetColor final : AGraphProvider::Command {
		SetColor(EntityId Id, const FLinearColor &Color);
	};

	struct SetSelectionType final : AGraphProvider::Command {
		SetSelectionType(EntityId Id, SelectionType NewType);
	};

	struct Move final : AGraphProvider::Command {
		Move(EntityId Id, const FVector &Delta, bool UpdateConnectedEdges);
	};

	struct Serialize final : AGraphProvider::Command {
		Serialize(EntityId Id, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);
	};

	struct Deserialize final : AGraphProvider::Command {
		Deserialize(
			EntityId GraphId,
			EntityId *NewVertexId,
			rapidjson::StringStream &JsonStringStream,
			rapidjson::Reader &Reader
		);
	};
}
