#pragma once

#include "VRControllerBase.h"
#include "VRControllerLeft.generated.h"

UCLASS(ClassGroup=(Custom))
class GRAPHS_API UVRControllerLeft final : public UVRControllerBase {
	GENERATED_BODY()
public:
	explicit UVRControllerLeft(const FObjectInitializer &ObjectInitializer);

	virtual void SetupInputBindings(APawn *Pawn, UInputComponent *PlayerInputComponent) const override;
	virtual void PlayHapticEffect(APlayerController *PlayerController, float Scale) const override;
	virtual void UpdateLaserPositionDirection(bool ShouldLerp) override;

	void SetTeleportationMode(bool Enable) const;
	void AdjustTeleportLaserLength(float Delta);
	FORCEINLINE const FVector &GetTeleportPoint() const;
private:
	UPROPERTY()
	UStaticMeshComponent *m_TeleportPreviewMesh;

	float m_TeleportLaserCurrentDistance = 150.0f;

	constexpr static FLinearColor m_TeleportLaserColor = FLinearColor(0.07451f, 0.360784f, 0.286275f);
	constexpr static float m_TeleportLaserLengthDeltaSpeed = 5.0f;
	constexpr static float m_TeleportLaserMinDistance = 50.0f;
	constexpr static float m_TeleportLaserMaxDistance = 1500.0f;
};
