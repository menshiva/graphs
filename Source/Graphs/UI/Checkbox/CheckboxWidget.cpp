#include "CheckboxWidget.h"
#include "Components/TextBlock.h"
#include "Graphs/UI/Button/ImageButtonWidget.h"

void UCheckboxWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	SetTicked(Ticked);
	if (CBButton)
		CBButton->SetOnClickEvent([&] { SetTicked(!Ticked); });
	if (CBText)
		CBText->SetText(Text);
}

void UCheckboxWidget::SetTicked(const bool IsTicked) {
	Ticked = IsTicked;
	if (CBButton)
		CBButton->SetImageVisibility(Ticked);
}

void UCheckboxWidget::SetOnClickEvent(TFunction<void()> &&ClickEvent) {
	if (CBButton) {
		CBButton->SetOnClickEvent([&, ClickEvent(MoveTemp(ClickEvent))] {
			SetTicked(!Ticked);
			ClickEvent();
		});
	}
}
