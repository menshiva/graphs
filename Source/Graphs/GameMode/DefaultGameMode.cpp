#include "DefaultGameMode.h"

#include "Graphs/Player/DefaultPlayer.h"
#include "Kismet/KismetSystemLibrary.h"

ADefaultGameMode::ADefaultGameMode(const FObjectInitializer &ObjectInitializer) : AGameModeBase(ObjectInitializer) {
	bPauseable = false;
	SetActorTickEnabled(false);
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;

	DefaultPawnClass = ADefaultPlayer::StaticClass();
}

void ADefaultGameMode::InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage) {
	Super::InitGame(MapName, Options, ErrorMessage);
	UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), "stat fps");
}
