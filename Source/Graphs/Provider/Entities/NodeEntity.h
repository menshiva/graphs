#pragma once

#include "Entity.h"
#include "NodeEntity.generated.h"

UCLASS()
class GRAPHS_API ANodeEntity final : public AEntity {
	GENERATED_BODY()
public:
	ANodeEntity() : AEntity(
		ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Engine/BasicShapes/Sphere")).Object,
		ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("/Game/Graphs/Materials/GraphMaterial")).Object
	) {}
};
