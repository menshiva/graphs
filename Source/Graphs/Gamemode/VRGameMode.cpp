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

void AVRGameMode::Logout(AController* Exiting) {
	Super::Logout(Exiting);
	UpdateStats();
}

void AVRGameMode::ToggleFPS() {
	FpsEnabled = !FpsEnabled;
	UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), "stat FPS");
	SaveConfig();
}

void AVRGameMode::ToggleUnit() {
	UnitEnabled = !UnitEnabled;
	UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), "stat Unit");
	SaveConfig();
}

void AVRGameMode::ToggleShowingCollisions() {
	ShowCollisions = !ShowCollisions;
	UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), "pxvis collision");
}

void AVRGameMode::UpdateStats() const {
	const auto World = GetWorld();
	if (FpsEnabled)
		UKismetSystemLibrary::ExecuteConsoleCommand(World, "stat fps");
	if (UnitEnabled)
		UKismetSystemLibrary::ExecuteConsoleCommand(World, "stat Unit");
	if (ShowCollisions)
		UKismetSystemLibrary::ExecuteConsoleCommand(World, "pxvis collision");
}
