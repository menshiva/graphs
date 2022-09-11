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
	void SelectVertexEntity(const EntityId &VertexId);
	void SelectEdgeEntity(const EntityId &EdgeId);
	void SelectGraphEntity(const EntityId &GraphId);

	TSet<EntityId> SelectedEntitiesToRemove;
};
