#pragma once

#include "../Tool.h"
#include "ToolRemover.generated.h"

UCLASS()
class GRAPHS_API UToolRemover final : public UTool {
	GENERATED_BODY()
public:
	UToolRemover();

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual bool OnRightTriggerAction(bool IsPressed) override;

	void RemoveSelectedEntities();
	void DeselectEntities();
private:
	struct GraphRemoveData {
		TSet<EntityId> VerticesToRemove;
		TSet<EntityId> EdgesToRemove;
	};

	TMap<EntityId, GraphRemoveData> GraphsRemoveData;
};
