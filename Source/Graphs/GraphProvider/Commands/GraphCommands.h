#pragma once

#include "Graphs/GraphProvider/GraphProvider.h"

namespace GraphCommands {
	struct Create final : AGraphProvider::Command {
		explicit Create(EntityId *NewId);
	};

	struct SetSelectionType final : AGraphProvider::Command {
		SetSelectionType(EntityId Id, SelectionType NewType);
	};

	struct Move final : AGraphProvider::Command {
		Move(EntityId Id, const FVector &Delta);
	};
}
