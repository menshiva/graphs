#include "MenuWidget.h"
#include "KeyboardWidget.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
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

void UMenuWidget::SetKeyboardVisibility(const bool Visible) {
	if (Visible == KeyboardVisible)
		return;
	if (Visible) {
		KeyboardVisible = true;
		Keyboard->SetVisibility(ESlateVisibility::Visible);
		Keyboard->PlayShowHideAnimation(EUMGSequencePlayMode::Forward, [] {});
	}
	else {
		Keyboard->PlayShowHideAnimation(
			EUMGSequencePlayMode::Reverse,
			[&] {
				Keyboard->SetVisibility(ESlateVisibility::Collapsed);
				KeyboardVisible = false;
			}
		);
	}
}

void UMenuWidget::SetHitEntity(const char *EntityName, FString &&EntityCaption) const {
	HitEntityTitle->SetText(FText::FromString(FString("Hit entity: ") + EntityName));
	HitEntityCaption->SetText(FText::FromString(MoveTemp(EntityCaption)));
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
