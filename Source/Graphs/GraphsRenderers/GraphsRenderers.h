#pragma once

#include "Renderers/GraphRenderer.h"
#include "GraphsRenderers.generated.h"

UCLASS()
class GRAPHS_API AGraphsRenderers final : public AActor {
	GENERATED_BODY()
public:
	AGraphsRenderers();

	EntityId GetEntityIdFromHitResult(const FHitResult &HitResult) const;
	bool ExecuteCommand(GraphsRenderersCommand &&Cmd, bool RedrawIfDirty = false) const;
	void RedrawIfDirty() const;

	void AddGraphRenderer(EntityId GraphId);
	void RemoveGraphRenderer(EntityId GraphId);
private:
	UPROPERTY()
	TSet<AGraphRenderer*> Renderers;

	TSparseArray<TWeakObjectPtr<AGraphRenderer>> IdsRenderersMappings;

	friend GraphsRenderersCommand;
};

using CommandImpl = TFunction<bool()>;
class GraphsRenderersCommand {
public:
	explicit GraphsRenderersCommand(CommandImpl &&Impl) : Implementation(MoveTemp(Impl)) {}
	virtual ~GraphsRenderersCommand() = default;
protected:
	FORCEINLINE static ES &GetESMut() {
		return ES::GetInstance();
	}

	FORCEINLINE bool ExecuteSubCommand(GraphsRenderersCommand &&Cmd, const bool RedrawIfDirty = false) const {
		return GraphsRenderers->ExecuteCommand(MoveTemp(Cmd), RedrawIfDirty);
	}

	FORCEINLINE AGraphRenderer *GetGraphRenderer(const EntityId GraphId) const {
		return GraphsRenderers->IdsRenderersMappings[GraphId.GetIndex()].Get();
	}
private:
	TWeakObjectPtr<const AGraphsRenderers> GraphsRenderers;
	CommandImpl Implementation;

	friend AGraphsRenderers;
};
