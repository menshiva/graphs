#pragma once

#include "Graphs/Player/ToolProvider/ToolProvider.h"
#include "Tool.generated.h"

UCLASS(Abstract)
class GRAPHS_API UTool : public UActorComponent, public RightControllerInputInterface {
	GENERATED_BODY()
public:
	UTool() {
		PrimaryComponentTick.bCanEverTick = false;
	}

	FORCEINLINE void SetupToolProvider(UToolProvider *Provider) { ToolProvider = Provider; }

	FORCEINLINE UToolProvider *GetToolProvider() const { return ToolProvider.Get(); }
	FORCEINLINE AGraphProvider *GetGraphProvider() const { return ToolProvider->GetGraphProvider(); }
	FORCEINLINE UVRControllerRight *GetVrRightController() const { return ToolProvider->GetVrPawn()->GetRightVrController(); }

	FORCEINLINE EntityId GetHitEntityId() const { return ToolProvider->GetHitEntityId(); }
	FORCEINLINE const FHitResult &GetHitEntityResult() const { return ToolProvider->GetHitResult(); }

	FORCEINLINE const FName &GetToolName() const { return ToolName; }

	virtual void OnAttach() {}

	virtual void OnDetach() {
		GetVrRightController()->SetToolStateEnabled(false);
	}

	virtual void TickTool() {}
private:
	TWeakObjectPtr<UToolProvider> ToolProvider;
	FName ToolName;
};
