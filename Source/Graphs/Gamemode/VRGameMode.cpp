#include "VRGameMode.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Kismet/KismetSystemLibrary.h"

AVRGameMode::AVRGameMode() {
	bPauseable = false;
	DefaultPawnClass = AVRPawn::StaticClass();
}

void AVRGameMode::InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage) {
	Super::InitGame(MapName, Options, ErrorMessage);
	UpdateStats();
}

void AVRGameMode::Logout(AController *Exiting) {
	Super::Logout(Exiting);
	UpdateStats();
}

void AVRGameMode::ToggleFPS() {
	FpsEnabled = !FpsEnabled;
	UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), "Stat FPS");
	SaveConfig();
}

void AVRGameMode::ToggleUnit() {
	UnitEnabled = !UnitEnabled;
	UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), "Stat Unit");
	SaveConfig();
}

void AVRGameMode::UpdateStats() const {
	const auto World = GetWorld();
	if (FpsEnabled)
		UKismetSystemLibrary::ExecuteConsoleCommand(World, "Stat FPS");
	if (UnitEnabled)
		UKismetSystemLibrary::ExecuteConsoleCommand(World, "Stat Unit");
}
