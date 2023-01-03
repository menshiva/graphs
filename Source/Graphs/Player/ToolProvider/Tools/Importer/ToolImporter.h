#pragma once

#include "../Tool.h"
#include "ToolImporter.generated.h"

UCLASS()
class GRAPHS_API UToolImporter final : public UTool {
	GENERATED_BODY()
public:
	UToolImporter();

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	static bool GetExportFolderContents(TArray<FString> &OutFilePaths);
	void ImportFromFile(const FString &FilePath);
	void DeselectImportedGraph();
private:
	FString ImportFromFileImpl(const FString &FilePath);

	EntityId ImportedGraphId = EntityId::NONE();
};
