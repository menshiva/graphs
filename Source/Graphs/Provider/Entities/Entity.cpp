#include "Entity.h"
#include "Graphs/Utils/Colors.h"

AEntity::AEntity(UStaticMesh *Mesh, UMaterial *Material) {
	PrimaryActorTick.bCanEverTick = false;
	SetMobility(EComponentMobility::Movable);
	if (Mesh != nullptr && Material != nullptr) {
		const auto Smc = GetStaticMeshComponent();
		Smc->SetStaticMesh(Mesh);
		MaterialInstance = Smc->CreateAndSetMaterialInstanceDynamicFromMaterial(0, Material);
		Smc->SetMaterial(0, MaterialInstance);
		Smc->SetEnableGravity(false);
		Smc->CanCharacterStepUpOn = ECB_No;
		Smc->SetCollisionProfileName("Graph");
		Smc->SetCastShadow(false);
	}
	else {
		MaterialInstance = nullptr;
	}
}

void AEntity::SetSelectionType(const SelectionType NewType) {
	if (Selection == NewType) return;
	Selection = NewType;
	switch (Selection) {
		case SelectionType::HIT:
		case SelectionType::SELECTED: {
			SetColor(ColorUtils::SelectionColor);
			break;
		}
		default: {
			SetColor(ColorUtils::GraphDefaultColor);
		}
	}
}
