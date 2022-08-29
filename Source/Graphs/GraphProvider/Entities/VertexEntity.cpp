#include "VertexEntity.h"
#include "Graphs/GraphProvider/GraphProvider.h"
#include "Engine/StaticMeshActor.h"
#include "Graphs/Utils/Consts.h"

VertexEntity::VertexEntity(const AGraphProvider &Provider): Entity(
	Provider.GetWorld()->SpawnActor<AStaticMeshActor>(),
	EntityType::VERTEX
) {
	const auto StaticMeshActor = Cast<AStaticMeshActor>(Actor);
	StaticMeshActor->PrimaryActorTick.bCanEverTick = false;
	StaticMeshActor->SetMobility(EComponentMobility::Movable);

	const auto Smc = StaticMeshActor->GetStaticMeshComponent();
	Smc->SetStaticMesh(Provider.GetVertexMesh());
	Smc->SetEnableGravity(false);
	Smc->CanCharacterStepUpOn = ECB_No;
	Smc->SetCollisionProfileName("Graph");
	Smc->SetCastShadow(false);

	const auto MaterialInstance = Smc->CreateAndSetMaterialInstanceDynamicFromMaterial(0, Provider.GetEntityActorMaterial());
	MaterialInstance->SetVectorParameterValue("Color", ColorConsts::GraphDefaultColor);
	Smc->SetMaterial(0, MaterialInstance);
}

void VertexEntity::SetActorColor(const FLinearColor &NewColor) const {
	const auto Smc = Cast<AStaticMeshActor>(Actor)->GetStaticMeshComponent();
	const auto MaterialInstance = Cast<UMaterialInstanceDynamic>(Smc->GetMaterial(0));
	MaterialInstance->SetVectorParameterValue("Color", NewColor);
}
