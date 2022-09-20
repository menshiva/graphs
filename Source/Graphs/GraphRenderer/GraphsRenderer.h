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
	RenderData GenerateVerticesRenderData() const;
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
		std::tuple<EntitySignature, bool, bool> &&ComponentDirty
	) {
		const auto Signature = std::get<0>(ComponentDirty);
		Renderer.ComponentsMeshDirty[Signature] =
			Renderer.ComponentsMeshDirty[Signature] | std::get<1>(ComponentDirty);
		Renderer.ComponentsCollisionDirty[Signature] =
			Renderer.ComponentsCollisionDirty[Signature] | std::get<2>(ComponentDirty);
	}
private:
	CommandImplementation Implementation;
	friend AGraphsRenderer;
};
