#include "ButtonWidget.h"
#include "Components/SizeBox.h"
#include "Components/ButtonSlot.h"
#include "Components/ScaleBox.h"

void UButtonWidget::NativePreConstruct() {
	Super::NativePreConstruct();

	if (BSizeBox) {
		BSizeBox->bOverride_WidthOverride = bOverride_WidthOverride;
		if (bOverride_WidthOverride)
			BSizeBox->SetWidthOverride(WidthOverride);
		BSizeBox->bOverride_HeightOverride = bOverride_HeightOverride;
		if (bOverride_HeightOverride)
			BSizeBox->SetHeightOverride(HeightOverride);
	}

	SetBackgroundColor(BackgroundColor);

	if (BScaleBox)
		Cast<UButtonSlot>(BScaleBox->Slot)->SetPadding(ContentPadding);
}

void UButtonWidget::NativeConstruct() {
	Super::NativeConstruct();
	if (BButton)
		BButton->OnPressed.AddDynamic(this, &UButtonWidget::HandleClicked);
}

void UButtonWidget::SetBackgroundColor(const FLinearColor &NewColor) {
	BackgroundColor = NewColor;

	if (BButton) {
		FButtonStyle Style = BButton->WidgetStyle;
		const FVector ColorVector{NewColor};
		Style.Normal.TintColor = NewColor;
		Style.Hovered.TintColor = FLinearColor(ColorVector * HoveredCoefficient);
		Style.Pressed.TintColor = FLinearColor(ColorVector * PressedCoefficient);
		BButton->SetStyle(Style);
	}
}

void UButtonWidget::SetHoveredPressedCoefficients(const float HoveredCoeff, const float PressedCoeff) {
	HoveredCoefficient = HoveredCoeff;
	PressedCoefficient = PressedCoeff;
	SetBackgroundColor(BackgroundColor);
}

void UButtonWidget::SetOnClickEvent(TFunction<void()> &&ClickEvent) {
	OnClick = ClickEvent;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UButtonWidget::HandleClicked() {
	if (OnClick)
		OnClick();
}
