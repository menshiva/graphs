#include "ExitPanelWidget.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/UI/Button/TextButtonWidget.h"

void UExitPanelWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	if (ExitButton)
		ExitButton->SetOnClickEvent([&] { GetOwningPlayerPawn<AVRPawn>()->QuitGame(); });
}
