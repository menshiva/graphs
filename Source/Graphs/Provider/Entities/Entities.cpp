#include "Entities.h"

AEntity::AEntity(const TCHAR *MeshAssetName, const TCHAR *MeshMaterialAssetName, const EntityType EntType) : Type(EntType) {
	PrimaryActorTick.bCanEverTick = false;
	SetMobility(EComponentMobility::Movable);

	const ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(MeshAssetName);
	const ConstructorHelpers::FObjectFinder<UMaterial> MeshMaterialAsset(MeshMaterialAssetName);

	const auto Smc = GetStaticMeshComponent();
	Smc->SetStaticMesh(MeshAsset.Object);
	Smc->SetMaterial(0, MeshMaterialAsset.Object);
	Smc->SetEnableGravity(false);
	Smc->CanCharacterStepUpOn = ECB_No;
	Smc->SetCollisionProfileName("Graph");
	Smc->SetCastShadow(false);
}

uint32 AEntity::GetId() const {
	return GetUniqueID();
}

EntityType AEntity::GetType() const {
	return Type;
}

bool AEntity::IsValid() const {
	return Type != EntityType::UNDENTIFIED;
}
