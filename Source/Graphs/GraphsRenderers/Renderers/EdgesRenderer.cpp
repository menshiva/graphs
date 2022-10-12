#include "EdgesRenderer.h"
#include "VerticesRenderer.h"
#include "Graphs/Utils/Utils.h"

DECLARE_CYCLE_STAT(TEXT("UEdgesRenderer::GenerateStartEndFaces"), STAT_UEdgesRenderer_GenerateStartEndFaces, GRAPHS_PERF_EDGES_RENDERER);
DECLARE_CYCLE_STAT(TEXT("UEdgesRenderer::GetSectionMeshForLOD"), STAT_UEdgesRenderer_GetSectionMeshForLOD, GRAPHS_PERF_EDGES_RENDERER);
DECLARE_CYCLE_STAT(TEXT("UEdgesRenderer::GetCollisionMesh"), STAT_UEdgesRenderer_GetCollisionMesh, GRAPHS_PERF_EDGES_RENDERER);

template <size_t FacePointsNum, typename Container>
void GenerateEdgeFaces(
	FVector FirstVertexPos, FVector SecondVertexPos,
	Container &OutVertices
) {
	SCOPE_CYCLE_COUNTER(STAT_UEdgesRenderer_GenerateStartEndFaces);

	constexpr static float StepAngle = 360.0f / static_cast<float>(FacePointsNum);
	const auto ForwardDir = (SecondVertexPos - FirstVertexPos).GetSafeNormal();

	// generate unit vector that is perpendicular to ForwardDir
	// this new vector is going to be a right directional vector
	const auto du = FVector::DotProduct(ForwardDir, FVector::UpVector);
	const auto df = FVector::DotProduct(ForwardDir, FVector::ForwardVector);
	const auto v1 = fabsf(du) < fabsf(df) ? FVector::UpVector : FVector::ForwardVector;
	auto RightDir = FVector::CrossProduct(v1, ForwardDir);
	check(FVector::Orthogonal(ForwardDir, RightDir));

	// offset vertex positions almost by vertex mesh scale
	const auto Offset = ForwardDir * UVerticesRenderer::MeshScale * 0.95f;
	FirstVertexPos += Offset;
	SecondVertexPos -= Offset;

	// generate faces
	RightDir = RightDir.RotateAngleAxis(StepAngle / 2.0f, ForwardDir);
	for (size_t i = 0; i < FacePointsNum; ++i) {
		const auto RotatedScaledVec = RightDir * UEdgesRenderer::MeshScale;
		OutVertices.Add(RotatedScaledVec + FirstVertexPos);
		OutVertices.Add(RotatedScaledVec + SecondVertexPos);
		RightDir = RightDir.RotateAngleAxis(StepAngle, ForwardDir);
	}
}

UEdgesRenderer::UEdgesRenderer() : URendererBase() {
	const static ConstructorHelpers::FObjectFinder<UMaterialInstance> MaterialAsset(
		TEXT("/Game/Graphs/Materials/EdgesMaterialInst")
	);
	MeshMaterial = MaterialAsset.Object;
}

void UEdgesRenderer::Initialize() {
	SetupMaterialSlot(0, "Edges Mesh Material", MeshMaterial);
	Super::Initialize();
}

bool UEdgesRenderer::GetSectionMeshForLOD(
	const int32 LODIndex,
	const int32 SectionId,
	FRuntimeMeshRenderableMeshData &MeshData
) {
	SCOPE_CYCLE_COUNTER(STAT_UEdgesRenderer_GetSectionMeshForLOD);
	check(LODIndex == 0 && SectionId == 0);

	RenderData TmpData;
	{
		FScopeLock Lock(&DataSyncRoot);
		TmpData.Positions = Data.Positions;
		TmpData.Colors = Data.Colors;
	}

	if (TmpData.Positions.Num() == 0)
		return false;

	check(TmpData.Positions.Num() % 2 == 0);
	check(TmpData.Positions.Num() == TmpData.Colors.Num());
	check(MeshData.Positions.Num() == 0);
	check(MeshData.Triangles.Num() == 0);

	const size_t EdgesNum = TmpData.Positions.Num() / 2;
	const size_t VerticesNum = MeshQuality * 2 * EdgesNum;
	const size_t IndicesNum = MeshQuality * 2 * 3 * EdgesNum;

	TArray<FVector> Positions;
	Positions.Reserve(VerticesNum);
	TArray<FColor> Colors;
	Colors.Reserve(VerticesNum);
	TArray<int32> Indices;
	Indices.Reserve(IndicesNum);

	size_t SkippedEdges = 0;
	for (size_t RdataI = 0; RdataI < EdgesNum; ++RdataI) {
		const auto &FirstVertexPos = TmpData.Positions[RdataI * 2];
		const auto &SecondVertexPos = TmpData.Positions[RdataI * 2 + 1];

		// do not generate edge if vertices have intersection
		if (FVector::DistSquared(FirstVertexPos, SecondVertexPos)
			<= 4 * UVerticesRenderer::MeshScale * UVerticesRenderer::MeshScale)
		{
			++SkippedEdges;
			continue;
		}

		const auto &FirstVertexColor = TmpData.Colors[RdataI * 2];
		const auto &SecondVertexColor = TmpData.Colors[RdataI * 2 + 1];

		const auto PrevVerticesNum = Positions.Num();
		GenerateEdgeFaces<MeshQuality>(FirstVertexPos, SecondVertexPos, Positions);

		for (uint32_t i = 0; i < MeshQuality; ++i) {
			Colors.Add(FirstVertexColor);
			Colors.Add(SecondVertexColor);
		}

		for (int32 i = 1; i < MeshQuality * 2; i += 2) {
			Indices.Append({
				PrevVerticesNum + i - 1,
				PrevVerticesNum + i,
				PrevVerticesNum + (i + 1) % (MeshQuality * 2)
			});
			Indices.Append({
				PrevVerticesNum + (i + 1) % (MeshQuality * 2),
				PrevVerticesNum + i,
				PrevVerticesNum + (i + 2) % (MeshQuality * 2)
			});
		}
	}

	const size_t SkippedVerticesNum = MeshQuality * 2 * SkippedEdges;
	const size_t SkippedIndicesNum = SkippedVerticesNum * 3;

	const size_t GeneratedVerticesNum = VerticesNum - SkippedVerticesNum;
	const size_t GeneratedIndicesNum = IndicesNum - SkippedIndicesNum;

	check(Positions.Num() == GeneratedVerticesNum);
	check(Colors.Num() == GeneratedVerticesNum);
	check(Indices.Num() == GeneratedIndicesNum);

	MeshData.Positions.Append(Positions);
	MeshData.Tangents.SetNum(GeneratedVerticesNum);
	MeshData.TexCoords.SetNum(GeneratedVerticesNum);
	MeshData.Colors.Append(Colors);
	MeshData.Triangles.Append(Indices);

	return true;
}

bool UEdgesRenderer::GetCollisionMesh(FRuntimeMeshCollisionData &CollisionData) {
	SCOPE_CYCLE_COUNTER(STAT_UEdgesRenderer_GetCollisionMesh);

	check(Data.Positions.Num() % 2 == 0);
	check(Data.StorageIds.Num() == Data.Positions.Num() / 2);
	check(CollisionData.Vertices.Num() == 0);
	check(CollisionData.Triangles.Num() == 0);
	check(CollisionData.CollisionSources.Num() == 0);

	if (Data.Positions.Num() == 0) {
		// hides collision if no data was provided
		GenerateEmptyCollision(CollisionData);
		return true;
	}

	const size_t VerticesNum = CollisionQuality * 2 * Data.StorageIds.Num();
	const size_t TrianglesNum = VerticesNum;
	const size_t SourcesNum = Data.StorageIds.Num();

	CollisionData.Vertices.Reserve(VerticesNum);
	CollisionData.Triangles.Reserve(TrianglesNum);
	CollisionData.CollisionSources.Reserve(SourcesNum);

	size_t SkippedEdges = 0;
	for (size_t RdataI = 0; RdataI < Data.StorageIds.Num(); ++RdataI) {
		const auto EdgeId = Data.StorageIds[RdataI];
		const auto &FirstVertexPos = Data.Positions[RdataI * 2];
		const auto &SecondVertexPos = Data.Positions[RdataI * 2 + 1];

		// do not generate edge if vertices have intersection
		if (FVector::DistSquared(FirstVertexPos, SecondVertexPos)
			<= 4 * UVerticesRenderer::MeshScale * UVerticesRenderer::MeshScale)
		{
			++SkippedEdges;
			continue;
		}

		const auto PrevVerticesNum = CollisionData.Vertices.Num();
		GenerateEdgeFaces<CollisionQuality>(FirstVertexPos, SecondVertexPos, CollisionData.Vertices);

		const int32_t TrianglesStart = CollisionData.Triangles.Num();
		for (int32 i = 1; i < CollisionQuality * 2; i += 2) {
			CollisionData.Triangles.Add(
				PrevVerticesNum + i - 1,
				PrevVerticesNum + i,
				PrevVerticesNum + (i + 1) % (CollisionQuality * 2)
			);
			CollisionData.Triangles.Add(
				PrevVerticesNum + (i + 1) % (CollisionQuality * 2),
				PrevVerticesNum + i,
				PrevVerticesNum + (i + 2) % (CollisionQuality * 2)
			);
		}
		const int32_t TrianglesEnd = CollisionData.Triangles.Num() - 1;

		CollisionData.CollisionSources.Emplace(
			TrianglesStart, TrianglesEnd,
			this, EdgeId,
			ERuntimeMeshCollisionFaceSourceType::Collision
		);
	}

	const size_t SkippedVerticesNum = CollisionQuality * 2 * SkippedEdges;
	check(CollisionData.Vertices.Num() == VerticesNum - SkippedVerticesNum);
	check(CollisionData.Triangles.Num() == TrianglesNum - SkippedVerticesNum);
	check(CollisionData.CollisionSources.Num() == SourcesNum - SkippedEdges);

	return true;
}
