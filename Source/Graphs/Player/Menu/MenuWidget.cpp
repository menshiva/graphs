#include "MenuWidget.h"
#include "../Pawn/VRPawn.h"
#include "Animation/UMGSequencePlayer.h"
#include "Components/Button.h"

void UMenuWidget::NativeConstruct() {
	Super::NativeConstruct();
	if (ExitButton)
		ExitButton->OnClicked.AddDynamic(Cast<AVRPawn>(GetOwningPlayerPawn()), &AVRPawn::QuitGame);
}

void UMenuWidget::PlayAnimation(
	UWidgetAnimation *Animation,
	const EUMGSequencePlayMode::Type Mode,
	TFunction<void()> &&OnEnd
) {
	UUserWidget::PlayAnimation(Animation, 0, 1, Mode);
	FTimerHandle AnimHandle;
	GetOwningPlayerPawn()->GetWorldTimerManager().SetTimer(
		AnimHandle,
		FTimerDelegate::CreateLambda(OnEnd),
		Animation->GetEndTime(),
		false
	);
}
