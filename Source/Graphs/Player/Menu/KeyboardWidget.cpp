#include "KeyboardWidget.h"
#include "Components/WidgetInteractionComponent.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/UI/Button/TextButtonWidget.h"

void UKeyboardWidget::NativeConstruct() {
	Super::NativeConstruct();
	const auto UiInteractor = GetOwningPlayerPawn<AVRPawn>()->GetRightVrController()->GetUiInteractor();

	const auto DigitButtons = {
		Button0, Button1, Button2,
		Button3, Button4, Button5,
		Button6, Button7, Button8,
		Button9
	};
	for (const auto DigitButton : DigitButtons) {
		DigitButton->SetOnClickEvent([UiInteractor, CharStr(DigitButton->GetTextStr())] {
			UiInteractor->SendKeyChar(CharStr);
		});
	}

	ButtonDel->SetOnClickEvent([UiInteractor] {
		UiInteractor->PressAndReleaseKey(EKeys::BackSpace);
	});

	ButtonDot->SetOnClickEvent([UiInteractor] {
		UiInteractor->PressAndReleaseKey(EKeys::Period);
	});
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
