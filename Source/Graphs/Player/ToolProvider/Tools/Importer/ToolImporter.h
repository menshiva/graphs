#pragma once

#include "../Tool.h"
#include "ToolImporter.generated.h"

UCLASS()
class GRAPHS_API UToolImporter final : public UTool {
	GENERATED_BODY()
public:
	UToolImporter();

	virtual void OnAttach() override;
	void OnImportClick(const FString &FilePath) const;

	void RefreshFileList() const;
	static bool ImportGraphFromFile(const FString &FilePath, GraphImportData &ImportData, FString &ErrorMessage);
};
