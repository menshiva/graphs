﻿#include "ToolImporter.h"
#include "ToolImporterPanelWidget.h"
#include "Graphs/GraphProvider/Commands/GraphCommands.h"
#include "Graphs/Utils/Consts.h"
#include "ThirdParty/rapidjson/error/en.h"

UToolImporter::UToolImporter() : UTool(
	"Import",
	TEXT("/Game/Graphs/UI/Icons/Import"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolImporterPanel")
) {}

void UToolImporter::OnAttach() {
	Super::OnAttach();
	RefreshFileList();
}

void UToolImporter::RefreshFileList() const {
	auto &FileManager = FPlatformFileManager::Get().GetPlatformFile();
	const auto ExportDirPath = FPaths::LaunchDir() + FileConsts::ExportDirName;

	if (!FileManager.CreateDirectoryTree(*ExportDirPath)) {
		GetToolPanel<UToolImporterPanelWidget>()->ShowErrorPanel("Failed to create export directory.");
		return;
	}

	TArray<FString> ImportFiles;
	FileManager.FindFiles(ImportFiles, *ExportDirPath, TEXT(".json"));

	const auto ImporterUI = GetToolPanel<UToolImporterPanelWidget>();
	ImporterUI->SetInputFiles(ImportFiles);
	ImporterUI->ShowImportPanel();
}

bool UToolImporter::ImportGraphFromFile(const FString &FilePath, FString &ErrorMessage) const {
	auto &FileManager = FPlatformFileManager::Get().GetPlatformFile();
	check(FilePath.EndsWith(".json"));

	if (!FileManager.FileExists(*FilePath)) {
		ErrorMessage = "File does not exist.";
		return false;
	}

	FString InputStr;
	if (!FFileHelper::LoadFileToString(InputStr, &FileManager, *FilePath)) {
		ErrorMessage = "Failed to read from file.";
		return false;
	}

	// Parse json with DOM style API.
	// https://rapidjson.org/md_doc_features.html#:~:text=DOM%20(Document%20Object%20Model)%20style%20API
	rapidjson::Document JsonDom;
	const FTCHARToUTF8 Convert(*InputStr);
	const rapidjson::ParseResult &ParseResult = JsonDom.Parse(Convert.Get(), Convert.Length());
	if (!ParseResult) {
		ErrorMessage =
			"JSON parse error: " + FString(GetParseError_En(ParseResult.Code()))
			+ " (" + FString::FromInt(ParseResult.Offset()) + ")";
		return false;
	}

	// Deserialize graph.
	EntityId NewGraphId = ENTITY_NONE;
	GetGraphProvider()->ExecuteCommand(GraphCommands::Deserialize(&NewGraphId, JsonDom, ErrorMessage));
	return NewGraphId != ENTITY_NONE;
}
