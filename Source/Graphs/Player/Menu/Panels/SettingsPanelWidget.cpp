#include "SettingsPanelWidget.h"
#include "Graphs/Gamemode/VRGameMode.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/UI/Checkbox/CheckboxWidget.h"
#include "Kismet/GameplayStatics.h"

void USettingsPanelWidget::NativeConstruct() {
	Super::NativePreConstruct();

	const auto Pawn = GetOwningPlayerPawn<AVRPawn>();
	const auto GameMode = Cast<AVRGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	CameraFadeAnimationTick->SetTicked(Pawn->IsCameraFadeAnimationEnabled());
	CameraFadeAnimationTick->SetOnClickEvent([Pawn] { Pawn->ToggleCameraFadeAnimation(); });

	FpsStatsTick->SetTicked(GameMode->IsFpsEnabled());
	FpsStatsTick->SetOnClickEvent([GameMode] { GameMode->ToggleFPS(); });

	UnitStatsTick->SetTicked(GameMode->IsUnitEnabled());
	UnitStatsTick->SetOnClickEvent([GameMode] { GameMode->ToggleUnit(); });
}
