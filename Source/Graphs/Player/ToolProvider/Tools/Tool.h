#pragma once

#include "Graphs/Player/ToolProvider/ToolProvider.h"
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

	FORCEINLINE bool SupportsType(const EntityType Type) const {
		return SupportedEntityTypesMask & static_cast<std::underlying_type_t<EntityType>>(Type);
	}

	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void TickTool() {}
protected:
	FORCEINLINE AGraphProvider *GetGraphProvider() const { return ToolProvider->GetGraphProvider(); }
	FORCEINLINE UVRControllerRight *GetVrRightController() const { return ToolProvider->GetVrPawn()->GetRightVrController(); }

	FORCEINLINE EntityId GetHitEntityId() const { return ToolProvider->GetHitEntityId(); }

	template <class WidgetClass>
	FORCEINLINE WidgetClass *GetToolPanel() const { return Cast<WidgetClass>(ToolPanel.Get()); }

	void SetSupportedEntityTypes(std::initializer_list<EntityType> &&Types);
private:
	TWeakObjectPtr<UToolProvider> ToolProvider;

	FName ToolName;

	UPROPERTY()
	UTexture2D *ToolImage;

	TSubclassOf<UToolWidget> ToolPanelClass;

	TWeakObjectPtr<UToolWidget> ToolPanel;

	std::underlying_type_t<EntityType> SupportedEntityTypesMask = 0;
};
