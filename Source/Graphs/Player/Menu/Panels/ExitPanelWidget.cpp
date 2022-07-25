#include "ExitPanelWidget.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/UI/Button/TextButtonWidget.h"

void UExitPanelWidget::NativeConstruct() {
	Super::NativeConstruct();
	if (ExitButton)
		ExitButton->OnClicked.AddDynamic(Cast<AVRPawn>(GetOwningPlayerPawn()), &AVRPawn::QuitGame);
}
