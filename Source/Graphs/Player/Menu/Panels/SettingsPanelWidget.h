#pragma once

#include "Blueprint/UserWidget.h"
#include "SettingsPanelWidget.generated.h"

UCLASS(Abstract, meta=(DisableNativeTick))
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API USettingsPanelWidget : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UCheckboxWidget *CameraFadeAnimationTick;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UCheckboxWidget *FpsStatsTick;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UCheckboxWidget *UnitStatsTick;
};
