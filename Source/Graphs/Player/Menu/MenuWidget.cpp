#include "MenuWidget.h"
#include "KeyboardWidget.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Graphs/UI/Button/ImageButtonWidget.h"

void UMenuWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	for (size_t i = 0; i < MenuButtonHolder->GetChildrenCount(); ++i) {
		const auto MenuButton = Cast<UImageButtonWidget>(MenuButtonHolder->GetChildAt(i));
		MenuButton->SetOnClickEvent([&, i] {
			SetActivePanel(i);
		});
		MenuButton->SetBackgroundColor(MenuButtonUnselectedColor);
	}
	SetActivePanel(0);
}

void UMenuWidget::ShowKeyboard(TFunction<void(TCHAR)> &&OnKey, TFunction<void()> &&OnDel) {
	Keyboard->SetOnKeyEvent(MoveTemp(OnKey));
	Keyboard->SetOnDelEvent(MoveTemp(OnDel));

	KeyboardVisible = true;
	Keyboard->SetVisibility(ESlateVisibility::Visible);
	Keyboard->PlayShowHideAnimation(EUMGSequencePlayMode::Forward, [] {});
}

void UMenuWidget::HideKeyboard() {
	Keyboard->PlayShowHideAnimation(
		EUMGSequencePlayMode::Reverse,
		[&] {
			Keyboard->SetVisibility(ESlateVisibility::Collapsed);
			KeyboardVisible = false;
		}
	);
}

void UMenuWidget::SetActivePanel(const size_t Index) const {
	if (ActivePanelSwitcher) {
		const size_t CurrentIndex = ActivePanelSwitcher->GetActiveWidgetIndex();
		const auto CurrentActiveMenuButton = Cast<UImageButtonWidget>(MenuButtonHolder->GetChildAt(CurrentIndex));
		CurrentActiveMenuButton->SetBackgroundColor(MenuButtonUnselectedColor);
		const auto NewActiveMenuButton = Cast<UImageButtonWidget>(MenuButtonHolder->GetChildAt(Index));
		NewActiveMenuButton->SetBackgroundColor(Background->BrushColor);
		ActivePanelSwitcher->SetActiveWidgetIndex(Index);
	}
}
