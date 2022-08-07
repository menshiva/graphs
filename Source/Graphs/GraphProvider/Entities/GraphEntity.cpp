#include "GraphEntity.h"
#include "Graphs/GraphProvider/GraphProvider.h"

GraphEntity::GraphEntity(const AGraphProvider &Provider) : Entity(
	Provider.GetWorld()->SpawnActor<AActor>(),
	EntityType::GRAPH
) {
	Actor->PrimaryActorTick.bCanEverTick = false;
	Actor->SetActorEnableCollision(false);
}
