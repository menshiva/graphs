#pragma once

#include "../Tool.h"
#include "Graphs/EntityStorage/Commands/VertexCommands.h"
#include "ToolEditor.generated.h"

UCLASS()
class GRAPHS_API UToolEditor final : public UTool {
	GENERATED_BODY()
public:
	UToolEditor();

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual bool OnRightTriggerAction(bool IsPressed) override;

	FORCEINLINE EntityId GetSelectedEntityId() const { return SelectedEntityId; }
	void SelectEntity(EntityId NewEntityId);

	bool InvertGraphColorful() const;
	void SetGraphColor(const FColor &Color) const;
	void RandomizeVerticesColors() const;

	void SetVertexColor(const FColor &Color) const;
	FORCEINLINE void SetVertexValue(const int32_t Value) const {
		VertexCommands::Mutable::SetValue(SelectedEntityId, Value);
	}

	void RestoreCache() const;
private:
	void RedrawSelectedGraph() const;
	void RedrawSelectedVertex() const;

	struct GraphCache {
		bool Colorful;
		TArray<FColor> VerticesColors;
	};

	struct VertexCache {
		FColor Color;
		int32_t Value;
	};

	EntityId SelectedEntityId = EntityId::NONE();

	GraphCache GraphEntityCache;
	VertexCache VertexEntityCache;
};
