#include "DefaultGameMode.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Kismet/KismetSystemLibrary.h"

ADefaultGameMode::ADefaultGameMode(const FObjectInitializer &ObjectInitializer) : AGameModeBase(ObjectInitializer) {
	bPauseable = false;
	DefaultPawnClass = AVRPawn::StaticClass();
}

void ADefaultGameMode::InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage) {
	Super::InitGame(MapName, Options, ErrorMessage);
	UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), "stat fps");
}
