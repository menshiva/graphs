#include "ToolButtonWidget.h"
#include "ImageButtonWidget.h"
#include "Components/TextBlock.h"

void UToolButtonWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	SetToolImage(ToolImage);
	SetToolNameText(ToolName);
}

void UToolButtonWidget::SetToolImage(UTexture2D *NewImage) const {
	if (TBButton)
		TBButton->SetImage(NewImage);
}

void UToolButtonWidget::SetToolNameText(const FText &NewText) const {
	if (TBText)
		TBText->SetText(NewText);
}

void UToolButtonWidget::SetOnClickEvent(TFunction<void()> &&ClickEvent) const {
	if (TBButton)
		TBButton->SetOnClickEvent(MoveTemp(ClickEvent));
}
