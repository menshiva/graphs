#include "NodeCommands.h"

NodeCommands::Create::Create(const FVector &Pos) : Position(Pos) {}

void NodeCommands::Create::Execute(AGraphProvider &Provider) {
	auto &NewEntity = Provider.CreateEntity<ANodeEntity>();
	NewEntity.SetActorLocation(Position);
}

NodeCommands::SetSelectionType::SetSelectionType(
	ANodeEntity *Node,
	const SelectionType NewType
) : Node(Node), Selection(NewType) {}

void NodeCommands::SetSelectionType::Execute(AGraphProvider &Provider) {
	Node->SetSelectionType(Selection);
}

// TODO
/*NodeCommands::Move::Move(const uint32 Id, const FVector &Pos): NodeEntityId(Id), Position(Pos) {}

void NodeCommands::Move::Execute(AGraphProvider &Provider) {
	Provider.GetEntity<ANodeEntity>(NodeEntityId)->SetActorLocation(Position);
}*/
