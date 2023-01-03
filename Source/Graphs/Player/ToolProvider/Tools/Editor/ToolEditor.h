#pragma once

#include "../Tool.h"
#include "Graphs/EntityStorage/Commands/VertexCommands.h"
#include "ToolEditor.generated.h"

UCLASS()
class GRAPHS_API UToolEditor final : public UTool {
	GENERATED_BODY()
public:
	UToolEditor();

	virtual void OnDetach() override;

	virtual bool OnRightTriggerAction(bool IsPressed) override;

	FORCEINLINE EntityId GetSelectedEntityId() const { return SelectedEntityId; }
	void SelectEntity(EntityId NewEntityId);

	bool InvertGraphColorful() const;
	void SetGraphColor(const FColor &Color) const;
	void RandomizeVerticesColors() const;

	void SetVertexColor(const FColor &Color) const;

	void SetEdgeWeight(float Weight) const;

	void RestoreCache() const;
private:
	void RedrawSelectedGraph() const;
	void RedrawSelectedVertex() const;

	struct VertexCache {
		FColor Color;
	};

	struct EdgeCache {
		float Weight;
	};

	struct GraphCache {
		bool Colorful;
		TArray<FColor> VerticesColors;
	};

	EntityId SelectedEntityId = EntityId::NONE();

	VertexCache VertexEntityCache;
	EdgeCache EdgeEntityCache;
	GraphCache GraphEntityCache;
};
