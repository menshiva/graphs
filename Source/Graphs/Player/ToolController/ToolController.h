#pragma once

#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/Provider/GraphProvider.h"
#include "ToolController.generated.h"

UCLASS()
class GRAPHS_API UToolController final : public UActorComponent {
	GENERATED_BODY()
public:
	UToolController();

	void SetupPawn(AVRPawn *Pawn);

	void OnEntityHitChanged(const UVRControllerBase *ControllerHit, const AEntity *Entity, bool IsHit) const;
protected:
	virtual void BeginPlay() override;
private:
	TWeakObjectPtr<AVRPawn> VrPawn;
	TWeakObjectPtr<AGraphProvider> Provider;
};
