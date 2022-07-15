#include "DefaultGameMode.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Kismet/KismetSystemLibrary.h"

ADefaultGameMode::ADefaultGameMode(const FObjectInitializer &ObjectInitializer) : AGameModeBase(ObjectInitializer) {
	bPauseable = false;
	DefaultPawnClass = AVRPawn::StaticClass();
}
