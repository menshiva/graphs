#include "MenuWidget.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Animation/UMGSequencePlayer.h"
#include "Components/Border.h"
#include "Components/WidgetSwitcher.h"
#include "WidgetComponents/ImageButtonWidget.h"

UMenuWidget::UMenuWidget(const FObjectInitializer &ObjectInitializer) : UUserWidget(ObjectInitializer) {
	bHasScriptImplementedTick = false;
}

void UMenuWidget::NativeConstruct() {
	Super::NativeConstruct();
	if (MenuHomeButton) {
		MenuButtons.Push(MenuHomeButton);
		MenuHomeButton->OnClicked.AddDynamic(this, &UMenuWidget::OnMenuHomeClick);
	}
	if (MenuSettingsButton) {
		MenuButtons.Push(MenuSettingsButton);
		MenuSettingsButton->OnClicked.AddDynamic(this, &UMenuWidget::OnMenuSettingsClick);
	}
	if (MenuExitButton) {
		MenuButtons.Push(MenuExitButton);
		MenuExitButton->OnClicked.AddDynamic(this, &UMenuWidget::OnMenuExitClick);
	}
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