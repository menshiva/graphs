#pragma once

#include "Graphs/GraphProvider/GraphProvider.h"

namespace VertexCommands {
	struct Create final : AGraphProvider::Command {
		Create(EntityId GraphId, EntityId *NewVertexId, const FVector &Position);
	};

	struct GetGraphId final : AGraphProvider::Command {
		GetGraphId(EntityId Id, EntityId &GraphId);
	};

	struct SetSelectionType final : AGraphProvider::Command {
		SetSelectionType(EntityId Id, SelectionType NewType);
	};

	struct Move final : AGraphProvider::Command {
		Move(EntityId Id, const FVector &Delta);
	};
}
