#pragma once

#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/GraphProvider/GraphProvider.h"
#include "ToolProvider.generated.h"

UCLASS()
class GRAPHS_API UToolProvider final : public UActorComponent, public RightControllerInputInterface {
	GENERATED_BODY()
public:
	UToolProvider();

	FORCEINLINE void SetupPawn(AVRPawn *Pawn) { VrPawn = Pawn; }

	FORCEINLINE EntityId GetHitEntityId() const { return HitEntityId; }
	FORCEINLINE const FHitResult &GetHitResult() const { return HitResult; }
	void SetHitResult(const FHitResult &NewHitResult);
	FORCEINLINE void ResetHitResult() { SetHitResult(FHitResult()); }

	virtual bool OnRightTriggerAction(bool IsPressed) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	virtual void BeginPlay() override;
private:
	void SetEntitySelectionType(SelectionType Selection) const;

	TWeakObjectPtr<AVRPawn> VrPawn;
	TWeakObjectPtr<AGraphProvider> GraphProvider;

	EntityId HitEntityId = ENTITY_NONE;
	FHitResult HitResult;
};
