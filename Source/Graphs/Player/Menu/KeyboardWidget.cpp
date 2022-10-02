#include "KeyboardWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Graphs/UI/Button/TextButtonWidget.h"

void UKeyboardWidget::SetOnKeyEvent(TFunction<void(TCHAR)> &&OnKey) const {
	const auto DigitButtons = GetDigitButtons();
	for (int i = 0; i < DigitButtons.Num(); ++i) {
		if (const auto DigitButton = DigitButtons[i]) {
			DigitButton->SetOnClickEvent([OnKey, i] {
				if (OnKey)
					OnKey('0' + i);
			});
		}
	}
}

void UKeyboardWidget::SetOnDelEvent(TFunction<void()> &&OnDel) const {
	if (ButtonDel) {
		ButtonDel->SetOnClickEvent([OnDel(MoveTemp(OnDel))] {
			if (OnDel)
				OnDel();
		});
	}
}

void UKeyboardWidget::PlayShowHideAnimation(const EUMGSequencePlayMode::Type Mode, TFunction<void()> &&OnEnd) {
	PlayAnimation(ShowHideAnimation, 0, 1, Mode);
	FTimerHandle AnimHandle;
	GetOwningPlayerPawn()->GetWorldTimerManager().SetTimer(
		AnimHandle,
		FTimerDelegate::CreateLambda(OnEnd),
		ShowHideAnimation->GetEndTime(),
		false
	);
}

TStaticArray<UTextButtonWidget*, 10> UKeyboardWidget::GetDigitButtons() const {
	TStaticArray<UTextButtonWidget*, 10> Out;
	Out[0] = Button0;
	Out[1] = Button1;
	Out[2] = Button2;
	Out[3] = Button3;
	Out[4] = Button4;
	Out[5] = Button5;
	Out[6] = Button6;
	Out[7] = Button7;
	Out[8] = Button8;
	Out[9] = Button9;
	return Out;
}
