#pragma once

#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/Provider/GraphProvider.h"
#include "ToolController.generated.h"

UCLASS()
class GRAPHS_API UToolController final : public UActorComponent, public LeftControllerInputInterface, public RightControllerInputInterface {
	GENERATED_BODY()
public:
	UToolController();

	FORCEINLINE void SetupPawn(AVRPawn *Pawn) { VrPawn = Pawn; }

	void OnEntityHitChanged(const UVRControllerBase *ControllerHit, AEntity *Entity, bool IsHit) const;

	virtual bool OnLeftTriggerAction(bool IsPressed) override;
	virtual bool OnRightTriggerAction(bool IsPressed) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	virtual void BeginPlay() override;
private:
	TWeakObjectPtr<AVRPawn> VrPawn;
	TWeakObjectPtr<AGraphProvider> Provider;
};
