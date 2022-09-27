#include "GraphsRenderers.h"

DECLARE_CYCLE_STAT(TEXT("AGraphsRenderer::ExecuteCommand"), STAT_AGraphsRenderer_ExecuteCommand, GRAPHS_PERF_GRAPHS_RENDERER);

AGraphsRenderers::AGraphsRenderers() {
	PrimaryActorTick.bCanEverTick = false;
}

EntityId AGraphsRenderers::GetEntityIdFromHitResult(const FHitResult &HitResult) const {
	if (HitResult.bBlockingHit)
		if (const auto GraphRenderer = Cast<AGraphRenderer>(HitResult.GetActor()))
			return GraphRenderer->GetEntityIdFromHitResult(HitResult);
	return EntityId::NONE();
}

void AGraphsRenderers::ExecuteCommand(GraphsRenderersCommand &&Cmd, const bool RedrawIfDirty) const {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderer_ExecuteCommand);

	Cmd.GraphsRenderers = this;
	Cmd.Implementation();

	if (RedrawIfDirty)
		this->RedrawIfDirty();
}

void AGraphsRenderers::RedrawIfDirty() const {
	for (const auto GraphRenderer : Renderers)
		GraphRenderer->RedrawIfDirty();
}

void AGraphsRenderers::AddGraphRenderer(const EntityId GraphId) {
	check(IsInGameThread());
	check(Renderers.Num() == IdsRenderersMappings.Num());

	const auto GraphRenderer = GetWorld()->SpawnActor<AGraphRenderer>();
	GraphRenderer->Initialize(GraphId);

	Renderers.Add(GraphRenderer);
	const auto CheckIdx = IdsRenderersMappings.Add(GraphRenderer);

	check(GraphRenderer->GetGraphId() == GraphId);
	check(GraphId == EntityId(CheckIdx, GRAPH));
	check(Renderers.Num() == IdsRenderersMappings.Num());
}

void AGraphsRenderers::RemoveGraphRenderer(const EntityId GraphId) {
	check(IsInGameThread());
	check(Renderers.Num() == IdsRenderersMappings.Num());

	const auto GraphRenderer = IdsRenderersMappings[GraphId.GetIndex()].Get();
	check(GraphRenderer->GetGraphId() == GraphId);

	IdsRenderersMappings.RemoveAt(GraphId.GetIndex());
	Renderers.Remove(GraphRenderer);

	GraphRenderer->Destroy();
	check(Renderers.Num() == IdsRenderersMappings.Num());
}
