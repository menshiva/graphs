#pragma once

#include "CoreMinimal.h"
#include "VRControllerBase.h"
#include "VRControllerLeft.generated.h"

UCLASS(ClassGroup=(Custom))
class GRAPHS_API UVRControllerLeft final : public UVRControllerBase {
	GENERATED_BODY()
public:
	explicit UVRControllerLeft(const FObjectInitializer &ObjectInitializer);

	virtual void SetupInputBindings(APawn *Pawn, UInputComponent *PlayerInputComponent) override;

	virtual void PlayHapticEffect(APlayerController *PlayerController) const override;
};
