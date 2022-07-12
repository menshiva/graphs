#include "MenuWidget.h"
#include "../Player/Pawn/VRPawn.h"
#include "Animation/UMGSequencePlayer.h"
#include "Components/Button.h"

void UMenuWidget::NativeConstruct() {
	Super::NativeConstruct();
	if (ExitButton)
		ExitButton->OnClicked.AddDynamic(Cast<AVRPawn>(GetOwningPlayerPawn()), &AVRPawn::QuitGame);
}

void UMenuWidget::PlayShowHideAnimation(const EUMGSequencePlayMode::Type Mode, TFunction<void()> &&OnEnd) {
	PlayAnimation(ShowHideAnimation, 0, 1, Mode);
	FTimerHandle AnimHandle;
	GetOwningPlayerPawn()->GetWorldTimerManager().SetTimer(
		AnimHandle,
		FTimerDelegate::CreateLambda(OnEnd),
		ShowHideAnimation->GetEndTime(),
		false
	);
}
