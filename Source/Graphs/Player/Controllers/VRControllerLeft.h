#pragma once

#include "CoreMinimal.h"
#include "VRControllerBase.h"
#include "Components/WidgetComponent.h"
#include "VRControllerLeft.generated.h"

UCLASS(ClassGroup=(Custom))
class GRAPHS_API UVRControllerLeft final : public UVRControllerBase {
	GENERATED_BODY()
public:
	explicit UVRControllerLeft(const FObjectInitializer &ObjectInitializer);

	virtual void SetupInputBindings(APawn *Pawn, UInputComponent *PlayerInputComponent) override;

	void ToggleTeleportationMode(bool Enable);
	void SetTeleportLaserDistance(float Delta);
	FVector GetTeleportPoint() const;

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction *ThisTickFunction
	) override;
private:
	UPROPERTY()
	UNiagaraComponent *TeleportLaser;

	bool TeleportLaserVisibility = false;
	float TeleportLaserCurrentDistance = 100.0f;

	inline const static FColor TeleportLaserColor = FColor::Blue;
	constexpr static float TeleportLaserDistanceSpeed = 3.0f;
	constexpr static float TeleportLaserMinDistance = 50.0f;
	constexpr static float TeleportLaserMaxDistance = 500.0f;
};
