#include "SettingsPanelWidget.h"
#include "Graphs/Gamemode/VRGameMode.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/UI/Checkbox/CheckboxWidget.h"
#include "Kismet/GameplayStatics.h"

void USettingsPanelWidget::NativePreConstruct() {
	Super::NativePreConstruct();

	if (CameraFadeAnimationTick)
		CameraFadeAnimationTick->SetOnClickEvent([&] { Pawn->ToggleCameraFadeAnimation(); });
	if (FpsStatsTick)
		FpsStatsTick->SetOnClickEvent([&] { Gamemode->ToggleFPS(); });
	if (UnitStatsTick)
		UnitStatsTick->SetOnClickEvent([&] { Gamemode->ToggleUnit(); });
}

void USettingsPanelWidget::NativeConstruct() {
	Super::NativePreConstruct();

	Pawn = GetOwningPlayerPawn<AVRPawn>();
	Gamemode = Cast<AVRGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	if (CameraFadeAnimationTick)
		CameraFadeAnimationTick->SetTicked(Pawn->IsCameraFadeAnimationEnabled());
	if (FpsStatsTick)
		FpsStatsTick->SetTicked(Gamemode->IsFpsEnabled());
	if (UnitStatsTick)
		UnitStatsTick->SetTicked(Gamemode->IsUnitEnabled());
}
