#include "MenuWidget.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Graphs/UI/Button/ImageButtonWidget.h"

void UMenuWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	for (size_t i = 0; i < MenuButtonHolder->GetChildrenCount(); ++i) {
		const auto MenuButton = Cast<UImageButtonWidget>(MenuButtonHolder->GetChildAt(i));
		MenuButton->SetOnClickEvent([&, i] {
			SetActivePanel(i);
		});
	}
}

void UMenuWidget::NativeConstruct() {
	Super::NativeConstruct();
	for (size_t i = 0; i < MenuButtonHolder->GetChildrenCount(); ++i) {
		const auto MenuButton = Cast<UImageButtonWidget>(MenuButtonHolder->GetChildAt(i));
		MenuButton->SetBackgroundColor(MenuButtonUnselectedColor);
	}
	SetActivePanel(0);
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
