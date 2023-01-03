#pragma once

#include "GameFramework/GameModeBase.h"
#include "VRGameMode.generated.h"

/**
 * Game mode class.
 *
 * This game mode can be configured to show the FPS and Unit stat in the top right corner of the screen.
 * These options are saved in the user preferences config file.
 */
UCLASS(Config = UserPreferences)
class GRAPHS_API AVRGameMode final : public AGameModeBase {
	GENERATED_BODY()
public:
	AVRGameMode();

	/**
	 * Initializes the game.
	 * This function is called when the game starts.
	 * It is responsible for showing the FPS and Unit stat if they are enabled in the user preferences.
	 */
	virtual void InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage) override;

	/**
	 * Notifies the game mode that a controller has logged out.
	 * This function is responsible for hiding the FPS and Unit stat if they are currently shown.
	 */
	virtual void Logout(AController *Exiting) override;

	FORCEINLINE bool IsFpsEnabled() const { return FpsEnabled; }
	FORCEINLINE bool IsUnitEnabled() const { return UnitEnabled; }

	void ToggleFPS();
	void ToggleUnit();
private:
	/**
	 * Updates the display of the statistics according to the current values of the FpsEnabled and
	 * UnitEnabled properties.
	 */
	void UpdateStats() const;

	UPROPERTY(Config)
	bool FpsEnabled = true;

	UPROPERTY(Config)
	bool UnitEnabled = false;
};
