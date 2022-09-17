#pragma once

#include "RuntimeMeshComponent.h"
#include "EntityStorage/EntityStorage.h"
#include "Components/VerticesRenderer.h"
#include "Components/EdgesRenderer.h"
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
	static void AddVertexColor(const VertexEntity &Vertex, TArray<FColor> &Out);
	RenderData GenerateVerticesRenderData() const;

	static void AddEdgeColor(
		const EdgeEntity &Edge,
		const VertexEntity &FirstVertex,
		const VertexEntity &SecondVertex,
		TArray<FColor> &Out
	);
	RenderData GenerateEdgesRenderData() const;

	// TODO: each graph on its own provider?

	std::bitset<GRAPH> ComponentsMeshDirty;
	std::bitset<GRAPH> ComponentsCollisionDirty;

	UPROPERTY()
	URuntimeMeshComponent *VerticesRuntimeMeshComponent;

	UPROPERTY()
	UVerticesRenderer *VerticesProvider;

	UPROPERTY()
	URuntimeMeshComponent *EdgesRuntimeMeshComponent;

	UPROPERTY()
	UEdgesRenderer *EdgesProvider;

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
