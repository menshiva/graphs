#pragma once

#include "VRControllerBase.h"
#include "VRControllerLeft.generated.h"

class LeftControllerInputInterface {
public:
	LeftControllerInputInterface() = default;
	virtual ~LeftControllerInputInterface() = default;

	virtual bool OnLeftMenuPressed() { return false; }

	virtual bool OnLeftTriggerAction(bool IsPressed) { return false; }
	virtual bool OnLeftGripAction(bool IsPressed) { return false; }

	virtual bool OnLeftThumbstickYAction(float Value) { return false; }
	virtual bool OnLeftThumbstickXAction(float Value) { return false; }

	virtual bool OnLeftThumbstickYAxis(float Value) { return false; }
	virtual bool OnLeftThumbstickXAxis(float Value) { return false; }
};

UCLASS()
class GRAPHS_API UVRControllerLeft final : public UVRControllerBase, public LeftControllerInputInterface {
	GENERATED_BODY()
public:
	explicit UVRControllerLeft(const FObjectInitializer &ObjectInitializer);

	virtual void SetupInputBindings(UInputComponent *Pic) override;

	virtual void SetState(ControllerState NewState) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	const FVector &GetTeleportLocation() const;
	void AdjustTeleportLaserLength(float Delta);

	virtual bool OnLeftMenuPressed() override;

	virtual bool OnLeftTriggerAction(bool IsPressed) override;
	virtual bool OnLeftGripAction(bool IsPressed) override;

	virtual bool OnLeftThumbstickYAction(float Value) override;
	virtual bool OnLeftThumbstickXAction(float Value) override;

	virtual bool OnLeftThumbstickYAxis(float Value) override;
	virtual bool OnLeftThumbstickXAxis(float Value) override;
private:
	UPROPERTY()
	UNiagaraComponent *TeleportLaser;

	UPROPERTY()
	UStaticMeshComponent *TeleportPreviewMesh;

	UPROPERTY()
	UNiagaraComponent *TeleportRing;

	FVector TeleportLocation;
	float TeleportLaserCurrentDistance = 150.0f;

	constexpr static FLinearColor TeleportLaserColor = FLinearColor(0.033345f, 0.161458f, 0.128114f);
	constexpr static float TeleportLaserLengthDeltaSpeed = 5.0f;
	constexpr static float TeleportLaserMinDistance = 25.0f;
	constexpr static float TeleportLaserMaxDistance = 1500.0f;
};
