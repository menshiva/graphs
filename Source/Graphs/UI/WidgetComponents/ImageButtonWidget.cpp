#include "ImageButtonWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/Image.h"
#include "Components/ScaleBox.h"

void UImageButtonWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	if (IBButton) {
		IBButton->SetBackgroundColor(BackgroundColor);
		IBButton->OnClicked = OnClicked;
		IBButton->OnHovered.AddDynamic(this, &UImageButtonWidget::OnHovered);
		IBButton->OnUnhovered.AddDynamic(this, &UImageButtonWidget::OnUnhovered);
		IBButton->OnPressed.AddDynamic(this, &UImageButtonWidget::OnPressed);
		IBButton->OnReleased.AddDynamic(this, &UImageButtonWidget::OnReleased);
	}
	if (IBScaleBox)
		Cast<UButtonSlot>(IBScaleBox->Slot)->SetPadding(ImagePadding);
	if (IBImage) {
		IBImage->SetBrushFromTexture(Image, true);
		IBImage->SetBrushTintColor(ImageColor);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UImageButtonWidget::OnHovered() {
	if (IBButton)
		IBButton->SetBackgroundColor(AddFloatToBackgroundColor(HoveredFloat));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UImageButtonWidget::OnUnhovered() {
	if (IBButton)
		IBButton->SetBackgroundColor(AddFloatToBackgroundColor(-HoveredFloat));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UImageButtonWidget::OnPressed() {
	if (IBButton)
		IBButton->SetBackgroundColor(AddFloatToBackgroundColor(PressedFloat));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UImageButtonWidget::OnReleased() {
	if (IBButton)
		IBButton->SetBackgroundColor(AddFloatToBackgroundColor(-PressedFloat));
}

FLinearColor UImageButtonWidget::AddFloatToBackgroundColor(const float Value) const {
	return {
		BackgroundColor.R + Value,
		BackgroundColor.G + Value,
		BackgroundColor.B + Value,
		BackgroundColor.A
	};
}
