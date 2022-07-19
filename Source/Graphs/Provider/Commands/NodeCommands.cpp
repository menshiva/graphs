#include "NodeCommands.h"

NodeCommands::Create::Create(uint32 *NewId, const FVector &Pos) : NewNodeEntityId(NewId), Position(Pos) {}

void NodeCommands::Create::Execute(AGraphProvider &Provider) {
	auto &NewEntity = Provider.CreateEntity<ANodeEntity>();
	NewEntity.SetActorLocation(Position);
	if (NewNodeEntityId)
		*NewNodeEntityId = NewEntity.GetId();
}

// TODO
/*NodeCommands::Move::Move(const uint32 Id, const FVector &Pos): NodeEntityId(Id), Position(Pos) {}

void NodeCommands::Move::Execute(AGraphProvider &Provider) {
	Provider.GetEntity<ANodeEntity>(NodeEntityId)->SetActorLocation(Position);
}*/
