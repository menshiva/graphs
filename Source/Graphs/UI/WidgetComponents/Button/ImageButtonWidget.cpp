#include "ImageButtonWidget.h"
#include "Components/Image.h"

UImageButtonWidget::UImageButtonWidget(const FObjectInitializer &ObjectInitializer) : UButtonWidget(ObjectInitializer) {}

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
