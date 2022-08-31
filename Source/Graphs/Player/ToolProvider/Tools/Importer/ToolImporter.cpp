#include "ToolImporter.h"
#include "DesktopPlatformModule.h"
#include "ToolImporterPanelWidget.h"
#include "Graphs/GraphProvider/Commands/GraphCommands.h"
#include "Graphs/Utils/Consts.h"

UToolImporter::UToolImporter() : UTool(
	"Import",
	TEXT("/Game/Graphs/UI/Icons/Import"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolImporterPanel")
) {}

void UToolImporter::OnAttach() {
	Super::OnAttach();
	GetToolPanel<UToolImporterPanelWidget>()->ShowImportPanel();
}

void UToolImporter::ImportGraph() const {
	if (!GEngine) {
		GetToolPanel<UToolImporterPanelWidget>()->ShowErrorPanel("Failed to get global UE4 engine pointer.");
		return;
	}

	if (!GEngine->GameViewport) {
		GetToolPanel<UToolImporterPanelWidget>()->ShowErrorPanel("Failed to get current game instance.");
		return;
	}

	const auto DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform) {
		GetToolPanel<UToolImporterPanelWidget>()->ShowErrorPanel("Failed to get desktop platform pointer.");
		return;
	}

	const auto OutputDir = FPaths::LaunchDir() + FileConsts::ExportDir;
	auto &FileManager = FPlatformFileManager::Get().GetPlatformFile();
	const auto DefaultPath = FileManager.DirectoryExists(*OutputDir)
		? OutputDir
		: FPaths::LaunchDir();

	// Open file picker dialog,
	TArray<FString> SelectedFilesPaths;
	if (!DesktopPlatform->OpenFileDialog(
		GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle(),
		"Import graph",
		DefaultPath,
		"",
		"JSON file|*.json",
		EFileDialogFlags::Type::None,
		SelectedFilesPaths
	)) {
		// dialog prompt was ignored
		return;
	}
	check(SelectedFilesPaths.Num() == 1);
	check(SelectedFilesPaths[0].EndsWith(".json"));

	EntityId NewGraphId = ENTITY_NONE;
	FString ErrorMsg;
	GetGraphProvider()->ExecuteCommand(GraphCommands::Import(
		&NewGraphId,
		SelectedFilesPaths[0],
		ErrorMsg
	));

	if (NewGraphId != ENTITY_NONE)
		GetToolPanel<UToolImporterPanelWidget>()->ShowSuccessPanel();
	else
		GetToolPanel<UToolImporterPanelWidget>()->ShowErrorPanel(ErrorMsg);
}
