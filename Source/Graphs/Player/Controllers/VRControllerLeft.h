#pragma once

#include "VRControllerBase.h"
#include "VRControllerLeft.generated.h"

UCLASS()
class GRAPHS_API UVRControllerLeft final : public UVRControllerBase {
	GENERATED_BODY()
public:
	explicit UVRControllerLeft(const FObjectInitializer &ObjectInitializer);

	virtual void SetupInputBindings(UInputComponent *Pic) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	UPROPERTY()
	UStaticMeshComponent *TeleportPreviewMesh;

	UPROPERTY()
	UNiagaraComponent *TeleportRing;

	constexpr static float TeleportLaserLengthDeltaSpeed = 5.0f;
	constexpr static float TeleportLaserDefaultLength = 150.0f;
	constexpr static float TeleportLaserMinLength = 25.0f;
	constexpr static float TeleportLaserMaxLength = 1500.0f;
};
