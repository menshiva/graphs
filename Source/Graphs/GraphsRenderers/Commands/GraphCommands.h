#pragma once

#include "../GraphsRenderers.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace GraphCommands {
	struct Create final : GraphsRenderersCommand {
		Create(EntityId *NewGraphId, bool Colorful);
	};

	struct FillFromImportData final : GraphsRenderersCommand {
		FillFromImportData(EntityId GraphId, const GraphImportData &ImportData);
	};

	struct Remove final : GraphsRenderersCommand {
		explicit Remove(EntityId GraphId);
	};

	struct RemoveAll final : GraphsRenderersCommand {
		RemoveAll();
	};

	struct UpdateCollisions final : GraphsRenderersCommand {
		explicit UpdateCollisions(EntityId GraphId);
	};

	struct SetHit final : GraphsRenderersCommand {
		SetHit(EntityId GraphId, bool IsHit);
	};

	struct SetOverrideColor final : GraphsRenderersCommand {
		SetOverrideColor(EntityId GraphId, const FColor &OverrideColor);
	};

	struct Move final : GraphsRenderersCommand {
		Move(EntityId GraphId, const FVector &Delta);
	};

	struct Rotate final : GraphsRenderersCommand {
		Rotate(EntityId GraphId, const FVector &Origin, float Angle);
	};

	namespace Consts {
		void Serialize(EntityId GraphId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);

		bool Deserialize(const FString &JsonStr, GraphImportData &ImportData, FString &ErrorMessage);

		FVector ComputeCenterPosition(EntityId GraphId);
	}
}
