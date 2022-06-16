#include "DefaultGameMode.h"
#include "Graphs/Player/Pawn/VRPlayerPawn.h"
#include "Kismet/KismetSystemLibrary.h"

ADefaultGameMode::ADefaultGameMode(const FObjectInitializer &ObjectInitializer) : AGameModeBase(ObjectInitializer) {
	bPauseable = false;
	SetActorTickEnabled(false);
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
	bAllowTickBeforeBeginPlay = false;

	DefaultPawnClass = AVRPlayerPawn::StaticClass();
	GameStateClass = nullptr;
	HUDClass = nullptr;
	GameSessionClass = nullptr;
	SpectatorClass = nullptr;
	ReplaySpectatorPlayerControllerClass = nullptr;
	ServerStatReplicatorClass = nullptr;
}

void ADefaultGameMode::InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage) {
	Super::InitGame(MapName, Options, ErrorMessage);
	UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), "stat fps");
}
