#pragma once

#include "Graphs/GraphProvider/GraphProvider.h"

namespace EdgeCommands {
	struct Create final : AGraphProvider::Command {
		Create(
			EntityId GraphId,
			EntityId FirstVertexId, EntityId SecondVertexId,
			EntityId *NewEdgeId
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
}
