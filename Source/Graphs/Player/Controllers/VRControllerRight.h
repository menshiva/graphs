#pragma once

#include "VRControllerBase.h"
#include "Components/WidgetInteractionComponent.h"
#include "VRControllerRight.generated.h"

UCLASS(ClassGroup=(Custom))
class GRAPHS_API UVRControllerRight final : public USceneComponent, public UVRControllerBase {
	GENERATED_BODY()
public:
	explicit UVRControllerRight(const FObjectInitializer &ObjectInitializer);

	virtual void SetupInputBindings(APawn *Pawn, UInputComponent *Pic) const override;

	virtual void PlayHapticEffect(APlayerController *PlayerController, float Scale) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetUiInteractionEnabled(bool Enabled);
	void UiLeftMouseButtonPress() const;
	void UiLeftMouseButtonRelease() const;
private:
	UFUNCTION()
	void OnUiHover(UWidgetComponent *WidgetComponent, UWidgetComponent *PreviousWidgetComponent);

	UPROPERTY()
	UWidgetInteractionComponent *m_UiInteractor;
};
