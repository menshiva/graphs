#pragma once

#include "VRControllerBase.h"
#include "VRControllerLeft.generated.h"

UCLASS(ClassGroup=(Custom))
class GRAPHS_API UVRControllerLeft final : public UVRControllerBase {
	GENERATED_BODY()
public:
	explicit UVRControllerLeft(const FObjectInitializer &ObjectInitializer);

	virtual void SetupInputBindings(APawn *Pawn, UInputComponent *PlayerInputComponent) override;

	void SetTeleportationMode(bool Enable) const;
	void AdjustTeleportLaserLength(float Delta);
	FORCEINLINE const FVector &GetTeleportPoint() const;

	virtual void UpdateLaserPositionDirection(bool ShouldLerp) override;
private:
	UPROPERTY()
	UStaticMeshComponent *m_TeleportPreviewMesh;

	float m_TeleportLaserCurrentDistance = 150.0f;

	constexpr static FLinearColor m_TeleportLaserColor = FLinearColor(0.1f, 1.0f, 0.2f);
	constexpr static float m_TeleportLaserLengthDeltaSpeed = 5.0f;
	constexpr static float m_TeleportLaserMinDistance = 50.0f;
	constexpr static float m_TeleportLaserMaxDistance = 1500.0f;
};
