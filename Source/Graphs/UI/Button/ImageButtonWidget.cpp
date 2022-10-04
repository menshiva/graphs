#include "ImageButtonWidget.h"
#include "Components/Image.h"

void UImageButtonWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	SetImage(Image);
	if (IBImage)
		IBImage->SetBrushTintColor(ImageColor);
}

void UImageButtonWidget::SetImage(UTexture2D *NewImage) const {
	if (IBImage)
		IBImage->SetBrushFromTexture(NewImage, true);
}

void UImageButtonWidget::SetImageVisibility(const bool Visible) const {
	if (IBImage)
		IBImage->SetVisibility(Visible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
