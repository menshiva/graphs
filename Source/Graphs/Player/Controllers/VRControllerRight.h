#pragma once

#include "VRControllerBase.h"
#include "VRControllerRight.generated.h"

UCLASS(ClassGroup=(Custom))
class GRAPHS_API UVRControllerRight final : public UVRControllerBase {
	GENERATED_BODY()
public:
	explicit UVRControllerRight(const FObjectInitializer &ObjectInitializer);

	virtual void SetupInputBindings(APawn *Pawn, UInputComponent *PlayerInputComponent) const override;
	virtual void PlayHapticEffect(APlayerController *PlayerController, float Scale) const override;
};
