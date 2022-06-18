#pragma once

#include "CoreMinimal.h"
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

	FORCEINLINE APlayerCameraManager *GetCameraManager() const;
	FORCEINLINE APlayerController *GetPlayerController() const;

	void TurnLeft();
	void TurnRight();

	void MoveY(float Speed);
	void MoveX(float Speed);

	void PrimaryActionPressed();
	void PrimaryActionReleased();

	void QuitGame();
protected:
	virtual void BeginPlay() override;
private:
	void CameraTeleportAnimation(TFunction<void()> &&DoAfterFadeIn);
	// 1.0 for FadeIn, 0.0 for FadeOut
	FORCEINLINE void FadeCamera(float Value) const;

	UPROPERTY()
	UCameraComponent *Camera;

	UPROPERTY()
	UVRControllerLeft *LeftController;

	UPROPERTY()
	UVRControllerRight *RightController;

	bool IsTeleporting = false;

	constexpr static float PlayerHeight = 111.0f;
	constexpr static float TurnAngle = 45.0f;
	constexpr static float SpeedCoefficient = 3.0f;
	constexpr static float ScreenFadeDuration = 0.3f;
};
