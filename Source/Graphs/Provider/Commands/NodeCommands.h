#pragma once

#include "Graphs/Provider/GraphProvider.h"
#include "Graphs/Provider/Entities/NodeEntity.h"

namespace NodeCommands {
	struct Create final : AGraphProvider::Command {
		explicit Create(const FVector &Pos);
		virtual void Execute(AGraphProvider &Provider) override;
	private:
		FVector Position;
	};

	struct SetSelectionType final : AGraphProvider::Command {
		explicit SetSelectionType(ANodeEntity *Node, SelectionType NewType);
		virtual void Execute(AGraphProvider &Provider) override;
	private:
		ANodeEntity *Node;
		SelectionType Selection;
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
