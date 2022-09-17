#include "Tool.h"
#include "ToolWidget.h"

UTool::UTool() {
	PrimaryComponentTick.bCanEverTick = false;
}

UTool::UTool(const FName &ToolName, const TCHAR *ToolImageAssetPath, const TCHAR *ToolPanelAssetPath): ToolName(ToolName) {
	PrimaryComponentTick.bCanEverTick = false;

	const ConstructorHelpers::FObjectFinder<UTexture2D> ToolImageAsset(ToolImageAssetPath);
	ToolImage = ToolImageAsset.Object;

	const ConstructorHelpers::FClassFinder<UToolWidget> ToolPanelAsset(ToolPanelAssetPath);
	ToolPanelClass = ToolPanelAsset.Class;
}

void UTool::SetToolPanel(UToolWidget *Panel) {
	ToolPanel = Panel;
	ToolPanel->SetupTool(this);
}

void UTool::SetSupportedEntities(std::initializer_list<EntitySignature> &&Signatures) {
	SupportedEntitiesMask = 0;
	for (const auto Signature : Signatures)
		SupportedEntitiesMask[Signature] = true;
}
