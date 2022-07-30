#pragma once

#include "Graphs/GraphProvider/GraphProvider.h"

namespace VertexCommands {
	struct Create final : AGraphProvider::Command {
		Create(EntityId GraphId, EntityId *NewVertexId, const FVector &Position);
	};

	struct Remove final : AGraphProvider::Command {
		explicit Remove(EntityId Id);
	};

	struct SetSelectionType final : AGraphProvider::Command {
		SetSelectionType(EntityId Id, SelectionType NewType);
	};
}
