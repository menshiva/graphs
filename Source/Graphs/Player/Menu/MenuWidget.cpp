#include "MenuWidget.h"
#include "../Pawn/VRPawn.h"
#include "Components/Button.h"

void UMenuWidget::NativeConstruct() {
	Super::NativeConstruct();
	if (ExitButton)
		ExitButton->OnClicked.AddDynamic(Cast<AVRPawn>(GetOwningPlayerPawn()), &AVRPawn::QuitGame);
}
