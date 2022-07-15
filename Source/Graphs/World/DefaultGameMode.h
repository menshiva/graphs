#pragma once

#include "GameFramework/GameModeBase.h"
#include "DefaultGameMode.generated.h"

UCLASS()
class GRAPHS_API ADefaultGameMode final : public AGameModeBase {
	GENERATED_BODY()
public:
	explicit ADefaultGameMode(const FObjectInitializer &ObjectInitializer);
};
