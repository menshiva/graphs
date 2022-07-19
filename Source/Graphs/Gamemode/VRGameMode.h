#pragma once

#include "GameFramework/GameModeBase.h"
#include "VRGameMode.generated.h"

UCLASS(Config = UserPreferences)
class GRAPHS_API AVRGameMode final : public AGameModeBase {
	GENERATED_BODY()
public:
	AVRGameMode();

	virtual void InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage) override;
	virtual void Logout(AController *Exiting) override;

	UFUNCTION()
	void ToggleFPS();

	UFUNCTION()
	void ToggleUnit();

	UPROPERTY(Config)
	bool FpsEnabled = true;

	UPROPERTY(Config)
	bool UnitEnabled = false;
private:
	void UpdateStats() const;
};
