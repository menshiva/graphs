#pragma once

#include "VRControllerBase.h"
#include "VRControllerLeft.generated.h"

class LeftControllerInputInterface {
public:
	LeftControllerInputInterface() = default;
	virtual ~LeftControllerInputInterface() = default;

	virtual bool OnLeftTriggerAction(bool IsPressed) { return false; }
};

UCLASS()
class GRAPHS_API UVRControllerLeft final : public UVRControllerBase, public LeftControllerInputInterface {
	GENERATED_BODY()
public:
	explicit UVRControllerLeft(const FObjectInitializer &ObjectInitializer);

	virtual void SetupInputBindings(UInputComponent *Pic) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual bool OnLeftTriggerAction(bool IsPressed) override;
private:
	UPROPERTY()
	UNiagaraComponent *TeleportLaser;

	UPROPERTY()
	UStaticMeshComponent *TeleportPreviewMesh;

	UPROPERTY()
	UNiagaraComponent *TeleportRing;

	FVector TeleportLocation;
	float TeleportLaserCurrentDistance = 150.0f;

	constexpr static float TeleportLaserLengthDeltaSpeed = 5.0f;
	constexpr static float TeleportLaserMinDistance = 25.0f;
	constexpr static float TeleportLaserMaxDistance = 1500.0f;
};
