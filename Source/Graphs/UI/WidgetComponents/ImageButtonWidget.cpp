#include "ImageButtonWidget.h"
#include "Components/Image.h"

void UImageButtonWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	if (IBImage) {
		IBImage->SetBrushFromTexture(Image, true);
		IBImage->SetBrushTintColor(ImageColor);
	}
}
