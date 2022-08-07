#pragma once

#include "Graphs/GraphProvider/GraphProvider.h"

namespace EdgeCommands {
	struct Create final : AGraphProvider::Command {
		Create(
			EntityId GraphId,
			EntityId FirstVertexId, EntityId SecondVertexId,
			EntityId *NewEdgeId,
			uint32_t EdgeDisplayId
		);
	};

	struct UpdateTransform final : AGraphProvider::Command {
		explicit UpdateTransform(EntityId Id);
	};

	struct GetGraphId final : AGraphProvider::Command {
		GetGraphId(EntityId Id, EntityId &GraphId);
	};

	struct SetSelectionType final : AGraphProvider::Command {
		SetSelectionType(EntityId Id, SelectionType NewType);
	};

	struct Move final : AGraphProvider::Command {
		Move(EntityId Id, const FVector &Delta, bool UpdateConnectedVertices);
	};
}
