#pragma once

#include "../Tool.h"
#include "ToolExporter.generated.h"

UCLASS()
class GRAPHS_API UToolExporter final : public UTool {
	GENERATED_BODY()
public:
	UToolExporter();

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual bool OnRightTriggerAction(bool IsPressed) override;
private:
	static FString GenerateJsonFileNameInDirectory(
		IPlatformFile &FileManager,
		const FString &DirPath,
		const FString &FilePrefix
	);

	static bool ExportGraph(
		EntityId GraphId,
		IPlatformFile &FileManager,
		const FString &OutputPath,
		FString &ErrorMessage
	);
};
