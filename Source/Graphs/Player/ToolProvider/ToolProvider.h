#pragma once

#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/GraphProvider/GraphProvider.h"
#include "ToolProvider.generated.h"

UCLASS()
class GRAPHS_API UToolProvider final : public UActorComponent, public RightControllerInputInterface {
	GENERATED_BODY()
public:
	explicit UToolProvider(const FObjectInitializer &ObjectInitializer);

	FORCEINLINE void SetupPawn(AVRPawn *Pawn) { VrPawn = Pawn; }

	FORCEINLINE AVRPawn *GetVrPawn() const { return VrPawn.Get(); }
	FORCEINLINE AGraphProvider *GetGraphProvider() const { return GraphProvider.Get(); }

	FORCEINLINE EntityId GetHitEntityId() const { return HitEntityId; }
	FORCEINLINE const FHitResult &GetHitResult() const { return HitResult; }
	void SetHitResult(const FHitResult &NewHitResult);
	FORCEINLINE void ResetHitResult() { SetHitResult(FHitResult()); }

	virtual bool OnRightTriggerAction(bool IsPressed) override;
	virtual bool OnRightThumbstickY(float Value) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void SetActiveTool(class UTool *NewTool);
protected:
	virtual void BeginPlay() override;
private:
	void SetEntitySelectionType(SelectionType Selection) const;

	template <class T>
	void RegisterTool(const FObjectInitializer &ObjectInitializer) {
		const auto Tool = ObjectInitializer.CreateDefaultSubobject<T>(this, T::GetToolName());
		Tool->SetupToolProvider(this);
		Tools.Push(Tool);
	}

	TWeakObjectPtr<AVRPawn> VrPawn;
	TWeakObjectPtr<AGraphProvider> GraphProvider;

	EntityId HitEntityId = ENTITY_NONE;
	FHitResult HitResult;

	UPROPERTY()
	TArray<UTool*> Tools;

	TWeakObjectPtr<UTool> ActiveTool;
};
