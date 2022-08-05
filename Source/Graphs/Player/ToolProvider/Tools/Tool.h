#pragma once

#include "Graphs/Player/ToolProvider/ToolProvider.h"
#include "Blueprint/UserWidget.h"
#include "Tool.generated.h"

UCLASS(Abstract)
class GRAPHS_API UTool : public UActorComponent, public RightControllerInputInterface {
	GENERATED_BODY()
public:
	UTool() {
		PrimaryComponentTick.bCanEverTick = false;
	}

	UTool(const TCHAR *ToolImageAssetPath, const TCHAR *ToolPanelAssetPath, const FName &ToolName) : ToolName(ToolName) {
		PrimaryComponentTick.bCanEverTick = false;

		const ConstructorHelpers::FObjectFinder<UTexture2D> ToolImageAsset(ToolImageAssetPath);
		ToolImage = ToolImageAsset.Object;

		const ConstructorHelpers::FClassFinder<UUserWidget> ToolPanelAsset(ToolPanelAssetPath);
		ToolPanelClass = ToolPanelAsset.Class;
	}

	FORCEINLINE void SetupToolProvider(UToolProvider *Provider) { ToolProvider = Provider; }

	FORCEINLINE UToolProvider *GetToolProvider() const { return ToolProvider.Get(); }
	FORCEINLINE AGraphProvider *GetGraphProvider() const { return ToolProvider->GetGraphProvider(); }
	FORCEINLINE UVRControllerRight *GetVrRightController() const { return ToolProvider->GetVrPawn()->GetRightVrController(); }

	FORCEINLINE EntityId GetHitEntityId() const { return ToolProvider->GetHitEntityId(); }
	FORCEINLINE const FHitResult &GetHitEntityResult() const { return ToolProvider->GetHitResult(); }

	FORCEINLINE UTexture2D *GetToolImage() const { return ToolImage; }
	FORCEINLINE const TSubclassOf<UUserWidget> &GetToolPanelClass() const { return ToolPanelClass; }
	FORCEINLINE const FName &GetToolName() const { return ToolName; }

	template <class WidgetClass>
	WidgetClass *GetToolPanel() const { return Cast<WidgetClass>(ToolPanel.Get()); }
	FORCEINLINE void SetToolPanel(UUserWidget *Panel) { ToolPanel = Panel; }

	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void TickTool() {}
private:
	TWeakObjectPtr<UToolProvider> ToolProvider;

	UPROPERTY()
	UTexture2D *ToolImage;

	TSubclassOf<UUserWidget> ToolPanelClass;

	FName ToolName;

	TWeakObjectPtr<UUserWidget> ToolPanel;
};
