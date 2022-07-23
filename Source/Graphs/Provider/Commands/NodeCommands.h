#pragma once

#include "Graphs/Provider/GraphProvider.h"

namespace NodeCommands {
	struct Create final : AGraphProvider::Command {
		explicit Create(uint32 *NewId, const FVector &Pos);
		virtual void Execute(AGraphProvider &Provider) override;
	private:
		uint32 *NewNodeEntityId;
		FVector Position;
	};

	// TODO
	/*struct Move final : AGraphProvider::Command {
		Move(const uint32 Id, const FVector &Pos);
		virtual void Execute(AGraphProvider &Provider) override;
	private:
		uint32 NodeEntityId;
		FVector Position;
	};*/
}
