#pragma once

#include "Blueprint/UserWidget.h"
#include "SettingsPanelWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API USettingsPanelWidget : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UCheckboxWidget *CameraFadeAnimationTick;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UCheckboxWidget *FpsStatsTick;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UCheckboxWidget *UnitStatsTick;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UCheckboxWidget *CollisionStatsTick;
private:
	TWeakObjectPtr<class AVRPawn> Pawn;
	TWeakObjectPtr<class AVRGameMode> Gamemode;
};
