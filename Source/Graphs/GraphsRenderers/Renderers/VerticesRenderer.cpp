#include "VerticesRenderer.h"
#include "Graphs/Utils/Utils.h"

DECLARE_CYCLE_STAT(TEXT("UVerticesRenderer::GetSectionMeshForLOD"), STAT_UVerticesRenderer_GetSectionMeshForLOD, GRAPHS_PERF_VERTICES_RENDERER);
DECLARE_CYCLE_STAT(TEXT("UVerticesRenderer::GetCollisionMesh"), STAT_UVerticesRenderer_GetCollisionMesh, GRAPHS_PERF_VERTICES_RENDERER);

UVerticesRenderer::UVerticesRenderer() : URendererBase() {}

bool UVerticesRenderer::GetSectionMeshForLOD(
	const int32 LODIndex,
	const int32 SectionId,
	FRuntimeMeshRenderableMeshData &MeshData
) {
	SCOPE_CYCLE_COUNTER(STAT_UVerticesRenderer_GetSectionMeshForLOD);
	check(LODIndex == 0 && SectionId == 0);

	RenderData TmpData;
	{
		FScopeLock Lock(&DataSyncRoot);
		TmpData.Positions = Data.Positions;
		TmpData.Colors = Data.Colors;
	}

	if (TmpData.Positions.Num() == 0)
		return false;

	check(TmpData.Positions.Num() == TmpData.Colors.Num());
	check(MeshData.Positions.Num() == 0);
	check(MeshData.Triangles.Num() == 0);

	const size_t VerticesNum = IcosahedronMesh.Vertices.size() * TmpData.Positions.Num();

	// mesh vertices
	{
		TArray<FVector> Positions;
		Positions.Reserve(VerticesNum);

		for (const auto &VertexPos : TmpData.Positions) {
			TArray<FVector, TFixedAllocator<IcosahedronMesh.Vertices.size()>> VertexMeshPositions(
				reinterpret_cast<const FVector*>(IcosahedronMesh.Vertices.data()),
				IcosahedronMesh.Vertices.size()
			);
			for (auto &VertexMeshPos : VertexMeshPositions)
				VertexMeshPos += VertexPos;
			Positions.Append(MoveTemp(VertexMeshPositions));
		}

		check(Positions.Num() == VerticesNum);
		MeshData.Positions.Append(Positions);
	}

	// mesh colors
	{
		TArray<FColor> Colors;
		Colors.Reserve(VerticesNum);

		for (const auto &VertexColor : TmpData.Colors) {
			TArray<FColor, TFixedAllocator<IcosahedronMesh.Vertices.size()>> VertexMeshColors;
			VertexMeshColors.Init(VertexColor, IcosahedronMesh.Vertices.size());
			Colors.Append(MoveTemp(VertexMeshColors));
		}

		check(Colors.Num() == VerticesNum);
		MeshData.Colors.Append(Colors);
	}

	// mesh indices
	{
		const size_t IndicesNum = IcosahedronMesh.Indices.size() * TmpData.Positions.Num();
		TArray<int32> Indices;
		Indices.Reserve(IndicesNum);

		for (size_t VerticesOffset = 0; VerticesOffset < VerticesNum; VerticesOffset += IcosahedronMesh.Vertices.size()) {
			TArray<int32, TFixedAllocator<IcosahedronMesh.Indices.size()>> VertexMeshIndices(
				IcosahedronMesh.Indices.data(),
				IcosahedronMesh.Indices.size()
			);
			if (VerticesOffset > 0)
				for (auto &Idx : VertexMeshIndices)
					Idx += VerticesOffset;
			Indices.Append(MoveTemp(VertexMeshIndices));
		}

		check(Indices.Num() == IndicesNum);
		MeshData.Triangles.Append(Indices);
	}

	MeshData.Tangents.SetNum(VerticesNum);
	MeshData.TexCoords.SetNum(VerticesNum);

	return true;
}

bool UVerticesRenderer::GetCollisionMesh(FRuntimeMeshCollisionData &CollisionData) {
	SCOPE_CYCLE_COUNTER(STAT_UVerticesRenderer_GetCollisionMesh);

	check(Data.StorageIds.Num() == Data.Positions.Num());
	check(Data.Positions.Num() == Data.StorageIds.Num());
	check(CollisionData.Vertices.Num() == 0);
	check(CollisionData.Triangles.Num() == 0);
	check(CollisionData.CollisionSources.Num() == 0);

	if (Data.Positions.Num() == 0) {
		// hides collision if no data was provided
		GenerateEmptyCollision(CollisionData);
		return true;
	}

	// collision vertices
	{
		const TArray<FVector, TFixedAllocator<IcosahedronCollision.Vertices.size()>> VertexCollisionPositions(
			reinterpret_cast<const FVector*>(IcosahedronCollision.Vertices.data()),
			IcosahedronCollision.Vertices.size()
		);

		const size_t VerticesNum = IcosahedronCollision.Vertices.size() * Data.Positions.Num();
		CollisionData.Vertices.Reserve(VerticesNum);

		for (const auto &VertexPos : Data.Positions) {
			for (const auto &VertexCollisionPos : VertexCollisionPositions)
				CollisionData.Vertices.Add(VertexCollisionPos + VertexPos);
		}

		check(CollisionData.Vertices.Num() == VerticesNum);
	}

	// collision triangles and sources
	{
		const TArray<int32, TFixedAllocator<IcosahedronCollision.Indices.size()>> VertexCollisionIndices(
			IcosahedronCollision.Indices.data(),
			IcosahedronCollision.Indices.size()
		);

		const size_t TrianglesNum = IcosahedronCollision.Indices.size() / 3 * Data.Positions.Num();
		const size_t SourcesNum = Data.Positions.Num();
		CollisionData.Triangles.Reserve(TrianglesNum);
		CollisionData.CollisionSources.Reserve(SourcesNum);

		size_t VerticesOffset = 0;
		for (const auto VertexId : Data.StorageIds) {
			const size_t StartTriangles = CollisionData.Triangles.Num();
			for (size_t j = 0; j < VertexCollisionIndices.Num(); j += 3) {
				CollisionData.Triangles.Add(
					VertexCollisionIndices[j + 0] + VerticesOffset,
					VertexCollisionIndices[j + 1] + VerticesOffset,
					VertexCollisionIndices[j + 2] + VerticesOffset
				);
			}
			const size_t EndTriangles = CollisionData.Triangles.Num() - 1;
			VerticesOffset += IcosahedronCollision.Vertices.size();

			CollisionData.CollisionSources.Emplace(
				StartTriangles, EndTriangles,
				this, VertexId,
				ERuntimeMeshCollisionFaceSourceType::Collision
			);
		}

		check(CollisionData.Triangles.Num() == TrianglesNum);
		check(CollisionData.CollisionSources.Num() == SourcesNum);
	}

	return true;
}
