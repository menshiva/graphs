#include "GraphRenderer.h"

DECLARE_CYCLE_STAT(TEXT("AGraphRenderer::RedrawIfDirty"), STAT_AGraphRenderer_RedrawIfDirty, GRAPHS_PERF_GRAPHS_RENDERER);
DECLARE_CYCLE_STAT(TEXT("AGraphRenderer::GenerateVerticesRenderData"), STAT_AGraphRenderer_GenerateVerticesRenderData, GRAPHS_PERF_GRAPHS_RENDERER);
DECLARE_CYCLE_STAT(TEXT("AGraphRenderer::GenerateEdgesRenderData"), STAT_AGraphRenderer_GenerateEdgesRenderData, GRAPHS_PERF_GRAPHS_RENDERER);

AGraphRenderer::AGraphRenderer(const FObjectInitializer &ObjectInitializer) {
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

void AGraphRenderer::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

	VerticesProvider->Shutdown();
	EdgesProvider->Shutdown();
}

void AGraphRenderer::MarkDirty(DirtyComponentInfo &&Info) {
	MeshDirty[Info.ComponentType] = MeshDirty[Info.ComponentType] | Info.MarkMesh;
	CollisionDirty[Info.ComponentType] = CollisionDirty[Info.ComponentType] | Info.MarkCollision;
}

void AGraphRenderer::Initialize(const EntityId InGraphId) {
	GraphId = InGraphId;

	VerticesProvider = NewObject<UVerticesRenderer>();
	VerticesRuntimeMeshComponent->Initialize(VerticesProvider);

	EdgesProvider = NewObject<UEdgesRenderer>();
	EdgesRuntimeMeshComponent->Initialize(EdgesProvider);
}

void AGraphRenderer::RedrawIfDirty() {
	if (MeshDirty == 0 && CollisionDirty == 0)
		return;

	SCOPE_CYCLE_COUNTER(STAT_AGraphRenderer_RedrawIfDirty);

	const bool UpdateVerticesLODs = MeshDirty[VERTEX];
	const bool UpdateVerticesCollision = CollisionDirty[VERTEX];

	const bool UpdateEdgesLODs = MeshDirty[EDGE];
	const bool UpdateEdgesCollision = CollisionDirty[EDGE];

	MeshDirty = 0;
	CollisionDirty = 0;

	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	if (UpdateVerticesLODs || UpdateVerticesCollision)
		VerticesProvider->SetRenderData(GenerateVerticesRenderData(Graph), UpdateVerticesLODs, UpdateVerticesCollision);

	if (UpdateEdgesLODs || UpdateEdgesCollision)
		EdgesProvider->SetRenderData(GenerateEdgesRenderData(Graph), UpdateEdgesLODs, UpdateEdgesCollision);
}

EntityId AGraphRenderer::GetEntityIdFromHitResult(const FHitResult &HitResult) const {
	auto HitEntityId = EntityId::NONE();
	if (HitResult.Component == VerticesRuntimeMeshComponent) {
		const auto HitSource = VerticesRuntimeMeshComponent->GetHitSource(HitResult.FaceIndex);
		if (HitSource.SourceProvider == VerticesProvider) {
			HitEntityId = EntityId(HitSource.SectionId, VERTEX);
			if (!ES::IsValid<VertexEntity>(HitEntityId))
				HitEntityId = EntityId::NONE();
		}
	}
	else if (HitResult.Component == EdgesRuntimeMeshComponent) {
		const auto HitSource = EdgesRuntimeMeshComponent->GetHitSource(HitResult.FaceIndex);
		if (HitSource.SourceProvider == EdgesProvider) {
			HitEntityId = EntityId(HitSource.SectionId, EDGE);
			if (!ES::IsValid<EdgeEntity>(HitEntityId))
				HitEntityId = EntityId::NONE();
		}
	}
	return HitEntityId;
}

RenderData AGraphRenderer::GenerateVerticesRenderData(const GraphEntity &Graph) const {
	SCOPE_CYCLE_COUNTER(STAT_AGraphRenderer_GenerateVerticesRenderData);

	RenderData RenderData;
	RenderData.StorageIds.SetNumUninitialized(Graph.Vertices.Num());
	RenderData.Positions.SetNumUninitialized(Graph.Vertices.Num());
	RenderData.Colors.SetNumUninitialized(Graph.Vertices.Num());

	ParallelFor(Graph.Vertices.Num(), [&Graph, &RenderData] (const int32 Idx) {
		const auto VertexId = Graph.Vertices[Idx];
		const auto &Vertex = ES::GetEntity<VertexEntity>(VertexId);

		RenderData.StorageIds[Idx] = VertexId.GetIndex();
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

RenderData AGraphRenderer::GenerateEdgesRenderData(const GraphEntity &Graph) const {
	SCOPE_CYCLE_COUNTER(STAT_AGraphRenderer_GenerateEdgesRenderData);

	RenderData RenderData;
	RenderData.StorageIds.SetNumUninitialized(Graph.Edges.Num());
	RenderData.Positions.SetNumUninitialized(Graph.Edges.Num() * 2);
	RenderData.Colors.SetNumUninitialized(Graph.Edges.Num() * 2);

	ParallelFor(Graph.Edges.Num(), [&Graph, &RenderData] (const int32 Idx) {
		const auto EdgeId = Graph.Edges[Idx];
		const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);
		const auto &FirstVertex = ES::GetEntity<VertexEntity>(Edge.ConnectedVertices[0]);
		const auto &SecondVertex = ES::GetEntity<VertexEntity>(Edge.ConnectedVertices[1]);

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

		RenderData.StorageIds[Idx] = EdgeId.GetIndex();

		RenderData.Positions[Idx * 2 + 0] = FirstVertex.Position;
		RenderData.Positions[Idx * 2 + 1] = SecondVertex.Position;

		RenderData.Colors[Idx * 2 + 0] = FirstColor;
		RenderData.Colors[Idx * 2 + 1] = SecondColor;
	});

	return RenderData;
}
