#pragma once

#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/GraphRenderer/GraphRenderer.h"
#include "ToolProvider.generated.h"

UCLASS()
class GRAPHS_API UToolProvider final : public UActorComponent, public RightControllerInputInterface {
	GENERATED_BODY()
public:
	explicit UToolProvider(const FObjectInitializer &ObjectInitializer);

	FORCEINLINE void SetupPawn(AVRPawn *Pawn) { VrPawn = Pawn; }

	FORCEINLINE AVRPawn *GetVrPawn() const { return VrPawn.Get(); }
	FORCEINLINE AGraphRenderer *GetGraphRenderer() const { return GraphRenderer.Get(); }
	FORCEINLINE const EntityStorage &GetEntityStorage() const { return GraphRenderer->GetEntityStorage(); }

	FORCEINLINE const EntityId &GetHitEntityId() const { return HitEntityId; }
	FORCEINLINE const FHitResult &GetHitResult() const { return HitResult; }
	void SetHitResult(const FHitResult &NewHitResult);
	FORCEINLINE void ResetHitResult() { SetHitResult(FHitResult()); }

	virtual bool OnRightTriggerAction(bool IsPressed) override;
	virtual bool OnRightThumbstickY(float Value) override;
	virtual bool OnRightThumbstickX(float Value) override;
	virtual bool OnRightThumbstickXAction(float Value) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void SetActiveTool(class UTool *NewTool);

	FORCEINLINE const TArray<UTool*> &GetTools() const { return Tools; }
protected:
	virtual void BeginPlay() override;
private:
	template <class T>
	void RegisterTool(const FObjectInitializer &ObjectInitializer, const FName &ToolObjectName) {
		const auto Tool = ObjectInitializer.CreateDefaultSubobject<T>(this, ToolObjectName);
		Tool->SetupToolProvider(this);
		Tools.Push(Tool);
	}

	void SetEntitySelection(EntitySelection NewSelection) const;

	TWeakObjectPtr<AVRPawn> VrPawn;
	TWeakObjectPtr<AGraphRenderer> GraphRenderer;

	EntityId HitEntityId;
	FHitResult HitResult;

	UPROPERTY()
	TArray<UTool*> Tools;

	TWeakObjectPtr<UTool> ActiveTool;
};
