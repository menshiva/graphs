#pragma once

#include "CoreMinimal.h"
#include "DefaultPlayer.generated.h"

UCLASS()
class GRAPHS_API ADefaultPlayer final : public APawn {
	GENERATED_BODY()
public:
	ADefaultPlayer();
protected:
	virtual void BeginPlay() override;
public:	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;
};
