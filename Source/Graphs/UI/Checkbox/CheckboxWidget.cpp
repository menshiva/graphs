#include "CheckboxWidget.h"
#include "Components/TextBlock.h"
#include "Graphs/UI/Button/ImageButtonWidget.h"

void UCheckboxWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	SetTicked(Ticked);
	if (CBText)
		CBText->SetText(Text);
}

void UCheckboxWidget::NativeConstruct() {
	Super::NativeConstruct();
	if (CBButton)
		CBButton->OnClicked.AddDynamic(this, &UCheckboxWidget::HandleClicked);
}

bool UCheckboxWidget::IsTicked() const {
	return Ticked;
}

void UCheckboxWidget::SetTicked(const bool IsTicked) {
	Ticked = IsTicked;
	if (CBButton)
		CBButton->SetImageVisibility(Ticked);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UCheckboxWidget::HandleClicked() {
	SetTicked(!Ticked);
	OnClicked.Broadcast();
}
