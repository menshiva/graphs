#pragma once

#include "CoreMinimal.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "Camera/CameraComponent.h"
#include "../Controllers/VRControllerLeft.h"
#include "../Controllers/VRControllerRight.h"
#include "VRPawn.generated.h"

UCLASS()
class GRAPHS_API AVRPawn final : public APawn {
	GENERATED_BODY()
public:
	explicit AVRPawn(const FObjectInitializer &ObjectInitializer);
	virtual void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;

	FORCEINLINE APlayerController *GetPlayerController() const;
	FORCEINLINE UVRControllerLeft *GetLeftController() const;
	FORCEINLINE UVRControllerRight *GetRightController() const;

	void TurnLeft();
	void TurnRight();
	void MoveY(float Speed);
	void MoveX(float Speed);
	void PrimaryAction();
	void QuitGame();
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	UHapticFeedbackEffect_Base *ControllerActionHapticEffect;

	UPROPERTY()
	UCameraComponent *Camera;

	UPROPERTY()
	UVRControllerLeft *LeftController;

	UPROPERTY()
	UVRControllerRight *RightController;

	constexpr static float SpeedCoefficient = 3.0f;
};
