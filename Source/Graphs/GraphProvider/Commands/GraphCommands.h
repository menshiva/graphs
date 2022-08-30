#pragma once

#include "Graphs/GraphProvider/GraphProvider.h"

namespace GraphCommands {
	struct Create final : AGraphProvider::Command {
		explicit Create(EntityId *NewId);
	};

	struct Remove final : AGraphProvider::Command {
		explicit Remove(EntityId Id);
	};

	struct SetColor final : AGraphProvider::Command {
		SetColor(EntityId Id, const FLinearColor &Color);
	};

	struct SetSelectionType final : AGraphProvider::Command {
		SetSelectionType(EntityId Id, SelectionType NewType);
	};

	struct Move final : AGraphProvider::Command {
		Move(EntityId Id, const FVector &Delta);
	};

	struct ComputeCenterPosition final : AGraphProvider::Command {
		ComputeCenterPosition(EntityId Id, FVector &Center);
	};

	struct Rotate final : AGraphProvider::Command {
		Rotate(EntityId Id, const FVector &Center, float Angle);
	};

	struct Import final : AGraphProvider::Command {
		enum class ResultType : uint8_t {
			ERROR,
			IGNORED,
			SUCCESS
		};

		Import(EntityId *NewId, ResultType &Result, FString &ErrorMessage);
	};

	struct Export final : AGraphProvider::Command {
		Export(EntityId Id, bool &Result, FString &ResultMessage);
	};
}
