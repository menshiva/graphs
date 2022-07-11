#pragma once

#include "VRControllerBase.h"
#include "Components/WidgetComponent.h"
#include "VRControllerLeft.generated.h"

UCLASS(ClassGroup=(Custom))
class GRAPHS_API UVRControllerLeft final : public USceneComponent, public UVRControllerBase {
	GENERATED_BODY()
public:
	explicit UVRControllerLeft(const FObjectInitializer &ObjectInitializer);

	virtual void SetupInputBindings(APawn *Pawn, UInputComponent *Pic) const override;

	virtual void PlayHapticEffect(APlayerController *PlayerController, float Scale) override;

	virtual void SetState(ControllerState NewState) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AdjustTeleportLaserLength(float Delta);

	void SpawnMainMenu(APawn *Pawn);
	void DestroyMainMenu();
private:
	UPROPERTY()
	UStaticMeshComponent *m_TeleportPreviewMesh;

	UPROPERTY()
	UWidgetComponent *m_MainMenu;

	TSubclassOf<UUserWidget> m_MainMenuWidgetClass;

	float m_TeleportLaserCurrentDistance = 150.0f;

	constexpr static FLinearColor m_TeleportLaserColor = FLinearColor(0.07451f, 0.360784f, 0.286275f);
	constexpr static float m_TeleportLaserLengthDeltaSpeed = 5.0f;
	constexpr static float m_TeleportLaserMinDistance = 50.0f;
	constexpr static float m_TeleportLaserMaxDistance = 1500.0f;
};
