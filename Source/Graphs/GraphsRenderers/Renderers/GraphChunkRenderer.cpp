#include "GraphChunkRenderer.h"

DECLARE_CYCLE_STAT(TEXT("AGraphChunkRenderer::RedrawIfDirty"), STAT_AGraphChunkRenderer_RedrawIfDirty, GRAPHS_PERF_GRAPH_CHUNK_RENDERER);
DECLARE_CYCLE_STAT(TEXT("AGraphChunkRenderer::GenerateVerticesRenderData"), STAT_AGraphChunkRenderer_GenerateVerticesRenderData, GRAPHS_PERF_GRAPH_CHUNK_RENDERER);
DECLARE_CYCLE_STAT(TEXT("AGraphChunkRenderer::GenerateEdgesRenderData"), STAT_AGraphChunkRenderer_GenerateEdgesRenderData, GRAPHS_PERF_GRAPH_CHUNK_RENDERER);

AGraphChunkRenderer::AGraphChunkRenderer(const FObjectInitializer &ObjectInitializer) {
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

void AGraphChunkRenderer::Initialize(const EntityId InGraphId) {
	const auto &Graph = ES::GetEntity<GraphEntity>(InGraphId);
	GraphId = InGraphId;

	VerticesProvider = NewObject<UVerticesRenderer>(this);
	if (VerticesToDraw.Num() > 0)
		VerticesProvider->SetInitRenderData(GenerateVerticesRenderData(Graph));
	VerticesRuntimeMeshComponent->Initialize(VerticesProvider);
	VerticesProvider->MarkCollisionDirty();

	EdgesProvider = NewObject<UEdgesRenderer>(this);
	if (EdgesToDraw.Num() > 0)
		EdgesProvider->SetInitRenderData(GenerateEdgesRenderData(Graph));
	EdgesRuntimeMeshComponent->Initialize(EdgesProvider);
	EdgesProvider->MarkCollisionDirty();
}

EntityId AGraphChunkRenderer::GetEntityIdFromHitResult(const FHitResult &HitResult) const {
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

bool AGraphChunkRenderer::AddVertex(const EntityId VertexId) {
	if (VerticesToDraw.Num() >= ChunkSize)
		return false;

	bool AlreadyInSet = false;
	VerticesToDraw.Add(VertexId, &AlreadyInSet);
	check(!AlreadyInSet);

	return true;
}

bool AGraphChunkRenderer::AddEdge(const EntityId EdgeId) {
	if (EdgesToDraw.Num() >= ChunkSize)
		return false;

	bool AlreadyInSet = false;
	EdgesToDraw.Add(EdgeId, &AlreadyInSet);
	check(!AlreadyInSet);

	return true;
}

void AGraphChunkRenderer::MarkAllVerticesDirty(const bool MarkMesh, const bool MarkCollision) {
	MeshDirty[VERTEX] = MeshDirty[VERTEX] | MarkMesh;
	CollisionDirty[VERTEX] = CollisionDirty[VERTEX] | MarkCollision;
}

void AGraphChunkRenderer::MarkAllEdgesDirty(const bool MarkMesh, const bool MarkCollision) {
	MeshDirty[EDGE] = MeshDirty[EDGE] | MarkMesh;
	CollisionDirty[EDGE] = CollisionDirty[EDGE] | MarkCollision;
}

bool AGraphChunkRenderer::RemoveVertex(const EntityId VertexId) {
	const bool Res = VerticesToDraw.Remove(VertexId) == 1;
	if (Res) // make set be a contiguous range
		VerticesToDraw.Compact();
	return Res;
}

bool AGraphChunkRenderer::RemoveEdge(const EntityId EdgeId) {
	const bool Res = EdgesToDraw.Remove(EdgeId) == 1;
	if (Res) // make set be a contiguous range
		EdgesToDraw.Compact();
	return Res;
}

void AGraphChunkRenderer::RedrawIfDirty() {
	SCOPE_CYCLE_COUNTER(STAT_AGraphChunkRenderer_RedrawIfDirty);

	if (MeshDirty == 0 && CollisionDirty == 0)
		return;

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

RenderData AGraphChunkRenderer::GenerateVerticesRenderData(const GraphEntity &Graph) const {
	SCOPE_CYCLE_COUNTER(STAT_AGraphChunkRenderer_GenerateVerticesRenderData);

	RenderData RenderData;
	RenderData.StorageIds.SetNumUninitialized(VerticesToDraw.Num());
	RenderData.Positions.SetNumUninitialized(VerticesToDraw.Num());
	RenderData.Colors.SetNumUninitialized(VerticesToDraw.Num());

	ParallelFor(VerticesToDraw.Num(), [&] (const int32 Idx) {
		const auto VertexId = VerticesToDraw[FSetElementId::FromInteger(Idx)];
		const auto &Vertex = ES::GetEntity<VertexEntity>(VertexId);
		check(Vertex.GraphId == GraphId);

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

RenderData AGraphChunkRenderer::GenerateEdgesRenderData(const GraphEntity &Graph) const {
	SCOPE_CYCLE_COUNTER(STAT_AGraphChunkRenderer_GenerateEdgesRenderData);

	RenderData RenderData;
	RenderData.StorageIds.SetNumUninitialized(EdgesToDraw.Num());
	RenderData.Positions.SetNumUninitialized(EdgesToDraw.Num() * 2);
	RenderData.Colors.SetNumUninitialized(EdgesToDraw.Num() * 2);

	ParallelFor(EdgesToDraw.Num(), [&] (const int32 Idx) {
		const auto EdgeId = EdgesToDraw[FSetElementId::FromInteger(Idx)];
		const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);
		check(Edge.GraphId == GraphId);
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
