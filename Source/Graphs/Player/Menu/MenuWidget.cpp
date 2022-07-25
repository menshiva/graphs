#include "MenuWidget.h"
#include "Animation/UMGSequencePlayer.h"
#include "Components/Border.h"
#include "Components/WidgetSwitcher.h"
#include "Graphs/UI/Button/ImageButtonWidget.h"

void UMenuWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	MenuButtons[0] = MenuHomeButton;
	MenuButtons[1] = MenuSettingsButton;
	MenuButtons[2] = MenuExitButton;
}

void UMenuWidget::NativeConstruct() {
	Super::NativeConstruct();
	if (MenuHomeButton)
		MenuHomeButton->OnClicked.AddDynamic(this, &UMenuWidget::OnMenuHomeClick);
	if (MenuSettingsButton)
		MenuSettingsButton->OnClicked.AddDynamic(this, &UMenuWidget::OnMenuSettingsClick);
	if (MenuExitButton)
		MenuExitButton->OnClicked.AddDynamic(this, &UMenuWidget::OnMenuExitClick);
}

void UMenuWidget::PlayShowHideAnimation(const EUMGSequencePlayMode::Type Mode, TFunction<void()> &&OnEnd) {
	PlayAnimation(ShowHideAnimation, 0, 1, Mode);
	FTimerHandle AnimHandle;
	GetOwningPlayerPawn()->GetWorldTimerManager().SetTimer(
		AnimHandle,
		FTimerDelegate::CreateLambda(OnEnd),
		ShowHideAnimation->GetEndTime(),
		false
	);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMenuWidget::OnMenuHomeClick() {
	SetActivePanel(0);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMenuWidget::OnMenuSettingsClick() {
	SetActivePanel(1);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMenuWidget::OnMenuExitClick() {
	SetActivePanel(2);
}

void UMenuWidget::SetActivePanel(const size_t Index) {
	if (ActivePanelSwitcher) {
		const size_t CurrentIndex = ActivePanelSwitcher->GetActiveWidgetIndex();
		MenuButtons[CurrentIndex]->SetBackgroundColor(MenuButtonUnselectedColor);
		MenuButtons[Index]->SetBackgroundColor(Background->BrushColor);
		ActivePanelSwitcher->SetActiveWidgetIndex(Index);
	}
}
