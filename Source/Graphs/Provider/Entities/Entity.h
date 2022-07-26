#pragma once

#include "Engine/StaticMeshActor.h"
#include "Entity.generated.h"

enum class SelectionType : uint8 {
	NONE,
	HIT,
	SELECTED
};

UCLASS(Abstract)
class GRAPHS_API AEntity : public AStaticMeshActor {
	GENERATED_BODY()
public:
	AEntity() = default;
	AEntity(UStaticMesh *Mesh, UMaterial *Material);

	FORCEINLINE void SetColor(const FLinearColor &NewColor) const {
		if (MaterialInstance)
			MaterialInstance->SetVectorParameterValue("Color", NewColor);
	}

	FORCEINLINE SelectionType GetSelectionType() const { return Selection; }
	void SetSelectionType(const SelectionType NewType);
private:
	UPROPERTY()
	UMaterialInstanceDynamic *MaterialInstance;

	SelectionType Selection = SelectionType::NONE;
};
