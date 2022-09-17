#pragma once

#include "RuntimeMeshComponent.h"
#include "EntityStorage/EntityStorage.h"
#include "Components/VerticesRenderer.h"
#include <bitset>
#include "GraphsRenderer.generated.h"

UCLASS()
class GRAPHS_API AGraphsRenderer final : public AActor {
	GENERATED_BODY()
public:
	explicit AGraphsRenderer(const FObjectInitializer &ObjectInitializer);
	virtual void OnConstruction(const FTransform &Transform) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	EntityId GetEntityIdFromHitResult(const FHitResult &HitResult) const;

	bool ExecuteCommand(GraphsRendererCommand &&Cmd, bool MarkDirty = false);
	void MarkDirty();
private:
	VerticesRenderData GenerateVerticesRenderData() const;

	// TODO: each graph on its own provider?

	std::bitset<GRAPH> ComponentsMeshDirty;
	std::bitset<GRAPH> ComponentsCollisionDirty;

	UPROPERTY()
	URuntimeMeshComponent *VerticesRuntimeMeshComponent;

	UPROPERTY()
	UVerticesRenderer *VerticesProvider;

	// TODO: edges provider

	friend GraphsRendererCommand;
};

using CommandImplementation = TFunction<bool(AGraphsRenderer &Renderer)>;
class GraphsRendererCommand {
public:
	explicit GraphsRendererCommand(CommandImplementation &&Impl) : Implementation(MoveTemp(Impl)) {}
	virtual ~GraphsRendererCommand() = default;
protected:
	FORCEINLINE static ES &ESMut() { return ES::GetInstance(); }

	// TODO: use 1 function (GetGraphRendererComponent(EntityId GraphId)) instead of functions below

	static void MarkRendererComponentDirty(
		AGraphsRenderer &Renderer,
		std::pair<EntitySignature, bool> &&ComponentDirty
	) {
		Renderer.ComponentsMeshDirty[ComponentDirty.first] = true;
		Renderer.ComponentsCollisionDirty[ComponentDirty.first] =
			Renderer.ComponentsCollisionDirty[ComponentDirty.first] | ComponentDirty.second;
	}
private:
	CommandImplementation Implementation;
	friend AGraphsRenderer;
};
