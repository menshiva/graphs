#include "ImageButtonWidget.h"
#include "Components/Image.h"

void UImageButtonWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	if (IBImage) {
		IBImage->SetBrushFromTexture(Image, true);
		IBImage->SetBrushTintColor(ImageColor);
	}
}

void UImageButtonWidget::SetImageVisibility(const bool Visible) const {
	if (IBImage)
		IBImage->SetVisibility(Visible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
