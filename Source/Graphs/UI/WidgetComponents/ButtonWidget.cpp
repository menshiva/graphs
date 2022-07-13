#include "ButtonWidget.h"
#include "Components/SizeBox.h"
#include "Components/ButtonSlot.h"
#include "Components/ScaleBox.h"

UButtonWidget::UButtonWidget(const FObjectInitializer &ObjectInitializer) : UUserWidget(ObjectInitializer) {
	bHasScriptImplementedTick = false;
}

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
		BButton->OnClicked.AddDynamic(this, &UButtonWidget::HandleClicked);
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

// ReSharper disable once CppMemberFunctionMayBeConst
void UButtonWidget::HandleClicked() {
	OnClicked.Broadcast();
}
