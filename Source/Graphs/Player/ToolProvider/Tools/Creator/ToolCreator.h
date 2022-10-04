#pragma once

#include "../Tool.h"
#include "ToolCreator.generated.h"

enum class CreationMode : uint8_t {
	VERTEX,
	EDGE,
	GRAPH
};

UCLASS()
class GRAPHS_API UToolCreator final : public UTool {
	GENERATED_BODY()
public:
	explicit UToolCreator(const FObjectInitializer &ObjectInitializer);

	FORCEINLINE CreationMode GetMode() const { return Mode; }
	void SetMode(CreationMode NewMode);

	FORCEINLINE bool IsGraphSelected() const { return SelectedGraphId != EntityId::NONE(); }
	void SetGraphSelection(EntityId GraphId);

	FORCEINLINE bool IsVertexSelected() const { return SelectedVertexId != EntityId::NONE(); }
	void SetVertexSelection(EntityId VertexId);

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void TickTool() override;

	virtual bool OnRightTriggerAction(bool IsPressed) override;
	virtual bool OnRightThumbstickY(float Value) override;
private:
	FORCEINLINE bool CheckVertexPreviewValidity() const {
		return !GetVrRightController()->IsInUiState()
		&& (Mode == CreationMode::GRAPH
			|| (Mode == CreationMode::VERTEX && SelectedGraphId != EntityId::NONE()));
	}

	bool CheckEdgePreviewValidity() const;

	UPROPERTY()
	UStaticMeshComponent *VertexPreviewMesh;

	UPROPERTY()
	UStaticMeshComponent *EdgePreviewMesh;

	CreationMode Mode = CreationMode::VERTEX;

	EntityId SelectedGraphId = EntityId::NONE();
	EntityId SelectedVertexId = EntityId::NONE();

	constexpr static float DefaultPreviewDistance = 300.0f;
	float PreviewDistance = DefaultPreviewDistance;
};
