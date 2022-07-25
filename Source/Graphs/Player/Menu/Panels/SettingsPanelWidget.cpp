#include "SettingsPanelWidget.h"
#include "Graphs/Gamemode/VRGameMode.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/UI/Checkbox/CheckboxWidget.h"
#include "Kismet/GameplayStatics.h"

void USettingsPanelWidget::NativeConstruct() {
	Super::NativeConstruct();
	const auto Pawn = Cast<AVRPawn>(GetOwningPlayerPawn());
	const auto Gamemode = Cast<AVRGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (CameraFadeAnimationTick) {
		CameraFadeAnimationTick->SetTicked(Pawn->IsCameraFadeAnimationEnabled());
		CameraFadeAnimationTick->OnClicked.AddDynamic(Pawn, &AVRPawn::ToggleCameraFadeAnimation);
	}
	if (FpsStatsTick) {
		FpsStatsTick->SetTicked(Gamemode->FpsEnabled);
		FpsStatsTick->OnClicked.AddDynamic(Gamemode, &AVRGameMode::ToggleFPS);
	}
	if (UnitStatsTick) {
		UnitStatsTick->SetTicked(Gamemode->UnitEnabled);
		UnitStatsTick->OnClicked.AddDynamic(Gamemode, &AVRGameMode::ToggleUnit);
	}
}
