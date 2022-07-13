#pragma once

#include "Blueprint/UserWidget.h"
#include "MenuWidget.generated.h"

UCLASS(Abstract)
class GRAPHS_API UMenuWidget final : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	void PlayShowHideAnimation(EUMGSequencePlayMode::Type Mode, TFunction<void()> &&OnEnd);
protected:
	UPROPERTY(BlueprintReadOnly, Transient, meta=(BindWidgetAnim))
	UWidgetAnimation *ShowHideAnimation;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UBorder *Background;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UWidgetSwitcher *ActivePanelSwitcher;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UImageButtonWidget *MenuHomeButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImageButtonWidget *MenuSettingsButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImageButtonWidget *MenuExitButton;
private:
	UFUNCTION()
	void OnMenuHomeClick();

	UFUNCTION()
	void OnMenuSettingsClick();

	UFUNCTION()
	void OnMenuExitClick();

	void SetActivePanel(size_t Index);

	TArray<UImageButtonWidget*> MenuButtons;
	constexpr static FLinearColor MenuButtonUnselectedColor = FLinearColor(0.03125f, 0.03125f, 0.03125f, 1.0f);
};
