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

	UTool(
		const FName &ToolName,
		const TCHAR *ToolImageAssetPath,
		const TCHAR *ToolPanelAssetPath,
		const std::initializer_list<EntityType> SupportedEntityTypes
	) : ToolName(ToolName) {
		PrimaryComponentTick.bCanEverTick = false;

		const ConstructorHelpers::FObjectFinder<UTexture2D> ToolImageAsset(ToolImageAssetPath);
		ToolImage = ToolImageAsset.Object;

		const ConstructorHelpers::FClassFinder<UUserWidget> ToolPanelAsset(ToolPanelAssetPath);
		ToolPanelClass = ToolPanelAsset.Class;

		for (const auto SupportedType : SupportedEntityTypes)
			SupportedEntityTypesMask |= static_cast<std::underlying_type_t<EntityType>>(SupportedType);
	}

	FORCEINLINE void SetupToolProvider(UToolProvider *Provider) { ToolProvider = Provider; }

	FORCEINLINE const FName &GetToolName() const { return ToolName; }
	FORCEINLINE UTexture2D *GetToolImage() const { return ToolImage; }
	FORCEINLINE const TSubclassOf<UUserWidget> &GetToolPanelClass() const { return ToolPanelClass; }

	FORCEINLINE void SetToolPanel(UUserWidget *Panel) { ToolPanel = Panel; }

	FORCEINLINE bool SupportsType(const EntityType Type) const {
		return SupportedEntityTypesMask & static_cast<std::underlying_type_t<EntityType>>(Type);
	}

	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void TickTool() {}
protected:
	FORCEINLINE UToolProvider *GetToolProvider() const { return ToolProvider.Get(); }
	FORCEINLINE AGraphProvider *GetGraphProvider() const { return ToolProvider->GetGraphProvider(); }
	FORCEINLINE UVRControllerRight *GetVrRightController() const { return ToolProvider->GetVrPawn()->GetRightVrController(); }

	FORCEINLINE EntityId GetHitEntityId() const { return ToolProvider->GetHitEntityId(); }

	template <class WidgetClass>
	WidgetClass *GetToolPanel() const { return Cast<WidgetClass>(ToolPanel.Get()); }
private:
	TWeakObjectPtr<UToolProvider> ToolProvider;

	FName ToolName;

	UPROPERTY()
	UTexture2D *ToolImage;

	TSubclassOf<UUserWidget> ToolPanelClass;

	TWeakObjectPtr<UUserWidget> ToolPanel;

	std::underlying_type_t<EntityType> SupportedEntityTypesMask = 0;
};
