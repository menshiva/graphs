#include "TextButtonWidget.h"
#include "Components/TextBlock.h"

void UTextButtonWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	if (TBText) {
		TBText->SetColorAndOpacity(TextColor);
		TBText->SetText(Text);
		FSlateFontInfo Font = TBText->Font;
		Font.Size = FontSize;
		Font.LetterSpacing = FontSize;
		Font.OutlineSettings.OutlineColor = OutlineColor;
		Font.OutlineSettings.OutlineSize = OutlineSize;
		TBText->SetFont(Font);
		TBText->SetJustification(ETextJustify::Center);
	}
}
