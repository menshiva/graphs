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

	FORCEINLINE bool IsFpsEnabled() const { return FpsEnabled; }
	FORCEINLINE bool IsUnitEnabled() const { return UnitEnabled; }

	void ToggleFPS();
	void ToggleUnit();
private:
	void UpdateStats() const;

	UPROPERTY(Config)
	bool FpsEnabled = true;

	UPROPERTY(Config)
	bool UnitEnabled = false;
};
