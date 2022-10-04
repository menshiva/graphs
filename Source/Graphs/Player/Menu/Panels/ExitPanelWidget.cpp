#include "ExitPanelWidget.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/UI/Button/TextButtonWidget.h"

void UExitPanelWidget::NativeConstruct() {
	Super::NativeConstruct();
	ExitButton->SetOnClickEvent([Pawn = GetOwningPlayerPawn<AVRPawn>()] { Pawn->QuitGame(); });
}
