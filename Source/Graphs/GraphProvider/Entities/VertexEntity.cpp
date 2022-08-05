#include "VertexEntity.h"
#include "Graphs/GraphProvider/GraphProvider.h"
#include "Engine/StaticMeshActor.h"
#include "Graphs/Utils/Colors.h"

VertexEntity::VertexEntity(const AGraphProvider &Provider): Entity(Provider.GetWorld()->SpawnActor<AStaticMeshActor>(), EntityType::VERTEX) {
	const auto NewActor = GetActor();
	NewActor->PrimaryActorTick.bCanEverTick = false;
	NewActor->SetMobility(EComponentMobility::Movable);
	const auto Smc = NewActor->GetStaticMeshComponent();
	Smc->SetStaticMesh(Provider.GetVertexMesh());
	const auto MaterialInstance = Smc->CreateAndSetMaterialInstanceDynamicFromMaterial(0, Provider.GetVertexMaterial());
	MaterialInstance->SetVectorParameterValue("Color", ColorUtils::GraphDefaultColor);
	Smc->SetMaterial(0, MaterialInstance);
	Smc->SetEnableGravity(false);
	Smc->CanCharacterStepUpOn = ECB_No;
	Smc->SetCollisionProfileName("Graph");
	Smc->SetCastShadow(false);
}

AStaticMeshActor *VertexEntity::GetActor() {
	return Cast<AStaticMeshActor>(Entity::GetActor());
}

void VertexEntity::SetActorColor(const FLinearColor &NewColor) {
	const auto MaterialInstance = Cast<UMaterialInstanceDynamic>(GetActor()->GetStaticMeshComponent()->GetMaterial(0));
	MaterialInstance->SetVectorParameterValue("Color", NewColor);
}
