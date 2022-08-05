#include "CheckboxWidget.h"
#include "Components/TextBlock.h"
#include "Graphs/UI/Button/ImageButtonWidget.h"

void UCheckboxWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	SetTicked(Ticked);
	if (CBButton) {
		CBButton->OnClick = [&] {
			SetTicked(!Ticked);
			OnClick();
		};
	}
	if (CBText)
		CBText->SetText(Text);
}

bool UCheckboxWidget::IsTicked() const {
	return Ticked;
}

void UCheckboxWidget::SetTicked(const bool IsTicked) {
	Ticked = IsTicked;
	if (CBButton)
		CBButton->SetImageVisibility(Ticked);
}
