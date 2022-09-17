#include "EdgesRenderer.h"
#include "VerticesRenderer.h"

// TODO: check performance with TArray and TArray with inline allocator
template <size_t FacePointsNum>
TStaticArray<FVector, FacePointsNum * 2> GenerateStartEndFaces(FVector FirstVertexPos, FVector SecondVertexPos) {
	constexpr static float StepAngle = 360.0f / static_cast<float>(FacePointsNum);

	const auto ForwardDir = (SecondVertexPos - FirstVertexPos).GetSafeNormal();

	// generate unit vector that is perpendicular to ForwardDir
	// this new vector is going to be a right directional vector
	const auto du = FVector::DotProduct(ForwardDir, FVector::UpVector);
	const auto df = FVector::DotProduct(ForwardDir, FVector::ForwardVector);
	const auto v1 = fabsf(du) < fabsf(df) ? FVector::UpVector : FVector::ForwardVector;
	auto RightDir = FVector::CrossProduct(v1, ForwardDir);
	check(FVector::Orthogonal(ForwardDir, RightDir));

	TStaticArray<FVector, FacePointsNum * 2> Out;

	// generate first face
	for (size_t i = 0; i < FacePointsNum; ++i) {
		Out[i] = RightDir * UEdgesRenderer::MeshScale;
		RightDir = RightDir.RotateAngleAxis(StepAngle, ForwardDir);
	}

	// offset vertex positions almost by vertex mesh scale
	const auto Offset = ForwardDir * UVerticesRenderer::MeshScale * 0.7f;
	FirstVertexPos += Offset;
	SecondVertexPos -= Offset;

	// copy to second face and apply offsets
	for (size_t i = FacePointsNum; i < FacePointsNum * 2; ++i) {
		Out[i] = Out[i - FacePointsNum];
		Out[i - FacePointsNum] += FirstVertexPos;
		Out[i] += SecondVertexPos;
	}

	return Out;
}

UEdgesRenderer::UEdgesRenderer() : URendererBase() {}

bool UEdgesRenderer::GetSectionMeshForLOD(
	const int32 LODIndex,
	const int32 SectionId,
	FRuntimeMeshRenderableMeshData &MeshData
) {
	check(LODIndex == 0 && SectionId == 0);

	RenderData TmpData;
	{
		FScopeLock Lock(&DataSyncRoot);
		if (Data.Positions.Num() == 0)
			return false;
		TmpData = Data;
	}

	check(TmpData.Positions.Num() % 2 == 0);
	check(TmpData.StorageIndices.Num() == TmpData.Positions.Num() / 2);
	check(TmpData.Positions.Num() == TmpData.Colors.Num());

	const size_t VerticesNum = MeshData.Positions.Num() + MeshQuality * 2 * TmpData.StorageIndices.Num();
	const size_t IndicesNum = MeshData.Triangles.Num() + MeshQuality * 2 * 3 * TmpData.StorageIndices.Num();

	MeshData.ReserveVertices(VerticesNum);
	MeshData.Triangles.Reserve(IndicesNum);

	size_t SkippedEdges = 0;
	for (size_t RdataI = 0; RdataI < TmpData.Positions.Num(); RdataI += 2) {
		const auto &FirstVertexPos = TmpData.Positions[RdataI];
		const auto &SecondVertexPos = TmpData.Positions[RdataI + 1];

		// do not generate edge if vertices have intersection
		if (FVector::DistSquared(FirstVertexPos, SecondVertexPos)
			<= 4 * UVerticesRenderer::MeshScale * UVerticesRenderer::MeshScale)
		{
			++SkippedEdges;
			continue;
		}

		const auto &FirstVertexColor = TmpData.Colors[RdataI];
		const auto &SecondVertexColor = TmpData.Colors[RdataI + 1];

		const auto VerticesOffset = MeshData.Positions.Num();
		const auto FacesPoints = GenerateStartEndFaces<MeshQuality>(FirstVertexPos, SecondVertexPos);
		for (const auto &P : FacesPoints)
			MeshData.Positions.Add(P);
		for (size_t i = 0; i < FacesPoints.Num(); ++i) {
			MeshData.Tangents.Add(FVector::UpVector, FVector::RightVector);
			MeshData.TexCoords.Add(FVector2D::UnitVector);
		}
		for (size_t i = 0; i < FacesPoints.Num() / 2; ++i)
			MeshData.Colors.Add(FirstVertexColor);
		for (size_t i = FacesPoints.Num() / 2; i < FacesPoints.Num(); ++i)
			MeshData.Colors.Add(SecondVertexColor);

		for (int32 i = 0; i < MeshQuality; ++i) {
			const int32 FirstFaceI = i;
			const int32 FirstFaceNextI = (i + 1) % MeshQuality;
			const int32 SecondFaceI = FirstFaceI + MeshQuality;
			const int32 SecondFaceNextI = FirstFaceNextI + MeshQuality;
			MeshData.Triangles.AddTriangle(
				VerticesOffset + FirstFaceI,
				VerticesOffset + SecondFaceI,
				VerticesOffset + FirstFaceNextI
			);
			MeshData.Triangles.AddTriangle(
				VerticesOffset + SecondFaceI,
				VerticesOffset + SecondFaceNextI,
				VerticesOffset + FirstFaceNextI
			);
		}
	}

	const size_t SkippedVerticesNum = MeshQuality * 2 * SkippedEdges;
	const size_t SkippedIndicesNum = SkippedVerticesNum * 3;
	check(MeshData.Positions.Num() == VerticesNum - SkippedVerticesNum);
	check(MeshData.Tangents.Num() == VerticesNum - SkippedVerticesNum);
	check(MeshData.TexCoords.Num() == VerticesNum - SkippedVerticesNum);
	check(MeshData.Colors.Num() == VerticesNum - SkippedVerticesNum);
	check(MeshData.Triangles.Num() == IndicesNum - SkippedIndicesNum);

	return true;
}

bool UEdgesRenderer::GetCollisionMesh(FRuntimeMeshCollisionData &CollisionData) {
	check(Data.Positions.Num() % 2 == 0);
	check(Data.StorageIndices.Num() == Data.Positions.Num() / 2);
	check(Data.Positions.Num() == Data.Colors.Num());

	if (Data.Positions.Num() == 0) {
		// hides collision if no data was provided
		GenerateEmptyCollision(CollisionData);
		return true;
	}

	const size_t VerticesNum = CollisionData.Vertices.Num() + CollisionQuality * 2 * Data.StorageIndices.Num();
	const size_t TrianglesNum = CollisionData.Triangles.Num() + CollisionQuality * 2 * Data.StorageIndices.Num();
	const size_t SourcesNum = CollisionData.CollisionSources.Num() + Data.StorageIndices.Num();

	CollisionData.Vertices.Reserve(VerticesNum);
	CollisionData.Triangles.Reserve(TrianglesNum);
	CollisionData.CollisionSources.Reserve(SourcesNum);

	size_t SkippedEdges = 0;
	for (size_t RdataI = 0; RdataI < Data.Positions.Num(); RdataI += 2) {
		const auto EdgeIdx = Data.StorageIndices[RdataI / 2];
		const auto &FirstVertexPos = Data.Positions[RdataI];
		const auto &SecondVertexPos = Data.Positions[RdataI + 1];

		// do not generate edge if vertices have intersection
		if (FVector::DistSquared(FirstVertexPos, SecondVertexPos)
			<= 4 * UVerticesRenderer::MeshScale * UVerticesRenderer::MeshScale)
		{
			++SkippedEdges;
			continue;
		}

		const auto VerticesOffset = CollisionData.Vertices.Num();
		const auto FacesPoints = GenerateStartEndFaces<CollisionQuality>(FirstVertexPos, SecondVertexPos);
		for (const auto &P : FacesPoints)
			CollisionData.Vertices.Add(P);

		const int32_t TrianglesStart = CollisionData.Triangles.Num();
		for (int32 i = 0; i < CollisionQuality; ++i) {
			const int32 FirstFaceI = i;
			const int32 FirstFaceNextI = (i + 1) % CollisionQuality;
			const int32 SecondFaceI = FirstFaceI + CollisionQuality;
			const int32 SecondFaceNextI = FirstFaceNextI + CollisionQuality;
			CollisionData.Triangles.Add(
				VerticesOffset + FirstFaceI,
				VerticesOffset + SecondFaceI,
				VerticesOffset + FirstFaceNextI
			);
			CollisionData.Triangles.Add(
				VerticesOffset + SecondFaceI,
				VerticesOffset + SecondFaceNextI,
				VerticesOffset + FirstFaceNextI
			);
		}
		const int32_t TrianglesEnd = CollisionData.Triangles.Num();

		CollisionData.CollisionSources.Emplace(
			TrianglesStart, TrianglesEnd,
			this, EdgeIdx,
			ERuntimeMeshCollisionFaceSourceType::Collision
		);
	}

	const size_t SkippedVerticesNum = CollisionQuality * 2 * SkippedEdges;
	check(CollisionData.Vertices.Num() == VerticesNum - SkippedVerticesNum);
	check(CollisionData.Triangles.Num() == TrianglesNum - SkippedVerticesNum);
	check(CollisionData.CollisionSources.Num() == SourcesNum - SkippedEdges);

	return true;
}
