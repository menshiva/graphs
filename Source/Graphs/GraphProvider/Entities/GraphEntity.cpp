#include "GraphEntity.h"
#include "Graphs/GraphProvider/GraphProvider.h"

GraphEntity::GraphEntity(const AGraphProvider &Provider) : Entity(Provider.GetWorld()->SpawnActor<AActor>(), EntityType::GRAPH) {
	const auto NewActor = Entity::GetActor();
	NewActor->PrimaryActorTick.bCanEverTick = false;
	NewActor->SetActorEnableCollision(false);
}
