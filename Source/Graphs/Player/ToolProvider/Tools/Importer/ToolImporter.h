#pragma once

#include "Graphs/Player/ToolProvider/Tools/Tool.h"
#include "ToolImporter.generated.h"

UCLASS()
class GRAPHS_API UToolImporter final : public UTool {
	GENERATED_BODY()
public:
	UToolImporter();

	virtual void OnAttach() override;

	void RefreshFileList() const;
	bool ImportGraphFromFile(const FString &FilePath, FString &ErrorMessage) const;
};
