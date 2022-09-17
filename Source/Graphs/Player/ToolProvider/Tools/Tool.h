#pragma once

#include "../ToolProvider.h"
#include "Tool.generated.h"

UCLASS(Abstract)
class GRAPHS_API UTool : public UActorComponent, public RightControllerInputInterface {
	GENERATED_BODY()
public:
	UTool();
	UTool(const FName &ToolName, const TCHAR *ToolImageAssetPath, const TCHAR *ToolPanelAssetPath);

	FORCEINLINE void SetupToolProvider(UToolProvider *Provider) { ToolProvider = Provider; }

	FORCEINLINE const FName &GetToolName() const { return ToolName; }
	FORCEINLINE UTexture2D *GetToolImage() const { return ToolImage; }
	FORCEINLINE const TSubclassOf<class UToolWidget> &GetToolPanelClass() const { return ToolPanelClass; }

	void SetToolPanel(UToolWidget *Panel);

	FORCEINLINE bool SupportsEntity(const EntityId &Id) const {
		return SupportedEntitiesMask[Id.Signature];
	}

	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void TickTool() {}
protected:
	FORCEINLINE UVRControllerRight *GetVrRightController() const { return ToolProvider->GetVrPawn()->GetRightVrController(); }
	FORCEINLINE const UToolProvider *GetToolProvider() const { return ToolProvider.Get(); }
	FORCEINLINE AGraphsRenderer *GetGraphsRenderer() const { return ToolProvider->GetGraphsRenderer(); }

	FORCEINLINE EntityId GetHitEntityId() const { return ToolProvider->GetHitEntityId(); }

	template <class WidgetClass>
	FORCEINLINE WidgetClass *GetToolPanel() const { return Cast<WidgetClass>(ToolPanel.Get()); }

	void SetSupportedEntities(std::initializer_list<EntitySignature> &&Signatures);
private:
	TWeakObjectPtr<UToolProvider> ToolProvider;

	FName ToolName;

	UPROPERTY()
	UTexture2D *ToolImage;

	TSubclassOf<UToolWidget> ToolPanelClass;
	TWeakObjectPtr<UToolWidget> ToolPanel;

	std::bitset<SIZE> SupportedEntitiesMask;
};
