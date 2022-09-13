﻿#pragma once

#include "Graphs/GraphProvider/GraphProvider.h"
#include "ThirdParty/rapidjson/prettywriter.h"
#include "ThirdParty/rapidjson/reader.h"

namespace GraphCommands {
	struct Create final : AGraphProvider::Command {
		explicit Create(EntityId *NewGraphId);
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

	struct Serialize final : AGraphProvider::Command {
		Serialize(EntityId Id, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);
	};

	struct Deserialize final : AGraphProvider::Command {
		Deserialize(
			EntityId *NewGraphId,
			rapidjson::StringStream &JsonStringStream,
			rapidjson::Reader &Reader,
			FString &ErrorMessage
		);
	};
}
