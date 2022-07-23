#include "TextButtonWidget.h"
#include "Components/TextBlock.h"

void UTextButtonWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	if (TBText) {
		TBText->SetColorAndOpacity(TextColor);
		TBText->SetText(Text);
		TBText->Font.Size = FontSize;
		TBText->Font.LetterSpacing = FontSize;
		TBText->SetJustification(ETextJustify::Center);
	}
}
