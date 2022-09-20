#include "GraphsRenderer.h"
#include "Commands/GraphCommands.h"

DECLARE_CYCLE_STAT(TEXT("AGraphsRenderer::ExecuteCommand"), STAT_AGraphsRenderer_ExecuteCommand, GRAPHS_PERF_GRAPHS_RENDERER);
DECLARE_CYCLE_STAT(TEXT("AGraphsRenderer::MarkDirty"), STAT_AGraphsRenderer_MarkDirty, GRAPHS_PERF_GRAPHS_RENDERER);
DECLARE_CYCLE_STAT(TEXT("AGraphsRenderer::GenerateVerticesRenderData"), STAT_AGraphsRenderer_GenerateVerticesRenderData, GRAPHS_PERF_GRAPHS_RENDERER);
DECLARE_CYCLE_STAT(TEXT("AGraphsRenderer::GenerateEdgesRenderData"), STAT_AGraphsRenderer_GenerateEdgesRenderData, GRAPHS_PERF_GRAPHS_RENDERER);

AGraphsRenderer::AGraphsRenderer(const FObjectInitializer &ObjectInitializer) {
	PrimaryActorTick.bCanEverTick = false;

	VerticesRuntimeMeshComponent = ObjectInitializer.CreateDefaultSubobject<URuntimeMeshComponent>(
		this,
		TEXT("VerticesRuntimeMeshComponent")
	);
	VerticesRuntimeMeshComponent->PrimaryComponentTick.bCanEverTick = false;
	VerticesRuntimeMeshComponent->SetMobility(EComponentMobility::Static);
	VerticesRuntimeMeshComponent->SetRuntimeMeshMobility(ERuntimeMeshMobility::Static);
	VerticesRuntimeMeshComponent->SetEnableGravity(false);
	VerticesRuntimeMeshComponent->CanCharacterStepUpOn = ECB_No;
	VerticesRuntimeMeshComponent->SetCollisionProfileName("Graph");
	VerticesRuntimeMeshComponent->SetCastShadow(false);

	EdgesRuntimeMeshComponent = ObjectInitializer.CreateDefaultSubobject<URuntimeMeshComponent>(
		this,
		TEXT("EdgesRuntimeMeshComponent")
	);
	EdgesRuntimeMeshComponent->PrimaryComponentTick.bCanEverTick = false;
	EdgesRuntimeMeshComponent->SetMobility(EComponentMobility::Static);
	EdgesRuntimeMeshComponent->SetRuntimeMeshMobility(ERuntimeMeshMobility::Static);
	EdgesRuntimeMeshComponent->SetEnableGravity(false);
	EdgesRuntimeMeshComponent->CanCharacterStepUpOn = ECB_No;
	EdgesRuntimeMeshComponent->SetCollisionProfileName("Graph");
	EdgesRuntimeMeshComponent->SetCastShadow(false);
}

void AGraphsRenderer::OnConstruction(const FTransform &Transform) {
	Super::OnConstruction(Transform);
	VerticesProvider = NewObject<UVerticesRenderer>(this);
	VerticesRuntimeMeshComponent->Initialize(VerticesProvider);

	EdgesProvider = NewObject<UEdgesRenderer>(this);
	EdgesRuntimeMeshComponent->Initialize(EdgesProvider);
}

void AGraphsRenderer::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	ExecuteCommand(GraphCommands::RemoveAll());
	Super::EndPlay(EndPlayReason);
}

EntityId AGraphsRenderer::GetEntityIdFromHitResult(const FHitResult &HitResult) const {
	auto HitEntityId = EntityId::NONE();

	if (HitResult.bBlockingHit && HitResult.GetActor() == this) {
		if (HitResult.Component == VerticesRuntimeMeshComponent) {
			HitEntityId = EntityId::Unhash(VerticesRuntimeMeshComponent->GetHitSource(HitResult.FaceIndex).SectionId);
			if (!ES::IsValid<VertexEntity>(HitEntityId))
				HitEntityId = EntityId::NONE();
		}
		else if (HitResult.Component == EdgesRuntimeMeshComponent) {
			HitEntityId = EntityId::Unhash(EdgesRuntimeMeshComponent->GetHitSource(HitResult.FaceIndex).SectionId);
			if (!ES::IsValid<EdgeEntity>(HitEntityId))
				HitEntityId = EntityId::NONE();
		}
	}

	return HitEntityId;
}

bool AGraphsRenderer::ExecuteCommand(GraphsRendererCommand &&Cmd, const bool MarkDirty) {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderer_ExecuteCommand);
	const auto Res = Cmd.Implementation(*this);
	if (MarkDirty)
		this->MarkDirty();
	return Res;
}

void AGraphsRenderer::MarkDirty() {
	if (ComponentsMeshDirty == 0 && ComponentsCollisionDirty == 0)
		return;

	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderer_MarkDirty);

	const bool UpdateVerticesLODs = ComponentsMeshDirty[VERTEX];
	const bool UpdateVerticesCollision = ComponentsCollisionDirty[VERTEX];
	ComponentsMeshDirty[VERTEX] = false;
	ComponentsCollisionDirty[VERTEX] = false;

	if (UpdateVerticesLODs || UpdateVerticesCollision)
		VerticesProvider->SetRenderData(GenerateVerticesRenderData(), UpdateVerticesLODs, UpdateVerticesCollision);

	const bool UpdateEdgesLODs = ComponentsMeshDirty[EDGE];
	const bool UpdateEdgesCollision = ComponentsCollisionDirty[EDGE];
	ComponentsMeshDirty[EDGE] = false;
	ComponentsCollisionDirty[EDGE] = false;

	if (UpdateEdgesLODs || UpdateEdgesCollision)
		EdgesProvider->SetRenderData(GenerateEdgesRenderData(), UpdateEdgesLODs, UpdateEdgesCollision);
}

RenderData AGraphsRenderer::GenerateVerticesRenderData() const {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderer_GenerateVerticesRenderData);
	const auto VerticesValidIds = ES::GetValidIndices<VertexEntity>();

	RenderData RenderData;
	RenderData.StorageIds.SetNumUninitialized(VerticesValidIds.Num());
	RenderData.Positions.SetNumUninitialized(VerticesValidIds.Num());
	RenderData.Colors.SetNumUninitialized(VerticesValidIds.Num());

	ParallelFor(VerticesValidIds.Num(), [&VerticesValidIds, &RenderData] (const int32 Idx) {
		const auto ValidId = VerticesValidIds[Idx];

		const auto &Vertex = ES::GetEntity<VertexEntity>(ValidId);
		const auto &Graph = ES::GetEntity<GraphEntity>(Vertex.GraphId);

		RenderData.StorageIds[Idx] = ValidId;
		RenderData.Positions[Idx] = Vertex.Position;
		RenderData.Colors[Idx] = Vertex.IsHit
			? ColorConsts::BlueColor
			: Vertex.OverrideColor != ColorConsts::OverrideColorNone
				? Vertex.OverrideColor
				: Graph.Colorful
					? Vertex.Color
					: ColorConsts::VertexDefaultColor;
	});

	return RenderData;
}

RenderData AGraphsRenderer::GenerateEdgesRenderData() const {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderer_GenerateEdgesRenderData);
	const auto EdgesValidIds = ES::GetValidIndices<EdgeEntity>();

	RenderData RenderData;
	RenderData.StorageIds.SetNumUninitialized(EdgesValidIds.Num());
	RenderData.Positions.SetNumUninitialized(EdgesValidIds.Num() * 2);
	RenderData.Colors.SetNumUninitialized(EdgesValidIds.Num() * 2);

	ParallelFor(EdgesValidIds.Num(), [&EdgesValidIds, &RenderData] (const int32 Idx) {
		const auto ValidId = EdgesValidIds[Idx];

		const auto &Edge = ES::GetEntity<EdgeEntity>(ValidId);
		const auto &Graph = ES::GetEntity<GraphEntity>(Edge.GraphId);
		const auto &FirstVertex = ES::GetEntity<VertexEntity>(Edge.VerticesIds[0]);
		const auto &SecondVertex = ES::GetEntity<VertexEntity>(Edge.VerticesIds[1]);

		auto FirstColor = FirstVertex.Color;
		auto SecondColor = SecondVertex.Color;
		if (Edge.IsHit) {
			FirstColor = ColorConsts::BlueColor;
			SecondColor = FirstColor;
		}
		else if (Edge.OverrideColor != ColorConsts::OverrideColorNone) {
			FirstColor = Edge.OverrideColor;
			SecondColor = FirstColor;
		}
		else if (!Graph.Colorful) {
			FirstColor = ColorConsts::VertexDefaultColor;
			SecondColor = FirstColor;
		}

		RenderData.StorageIds[Idx] = ValidId;

		RenderData.Positions[Idx * 2 + 0] = FirstVertex.Position;
		RenderData.Positions[Idx * 2 + 1] = SecondVertex.Position;

		RenderData.Colors[Idx * 2 + 0] = FirstColor;
		RenderData.Colors[Idx * 2 + 1] = SecondColor;
	});

	return RenderData;
}
