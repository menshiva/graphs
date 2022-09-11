﻿#include "GraphRenderer.h"
#include "ProceduralMeshComponent.h"
#include "MeshFactory.h"

enum Section : int32 {
	VERTICES = 0,
	EDGES
};

AGraphRenderer::AGraphRenderer() {
	PrimaryActorTick.bCanEverTick = false;

	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProcMesh");
	ProcMesh->PrimaryComponentTick.bCanEverTick = false;
	ProcMesh->SetMobility(EComponentMobility::Static);
	ProcMesh->SetEnableGravity(false);
	ProcMesh->CanCharacterStepUpOn = ECB_No;
	ProcMesh->SetCollisionProfileName("Graph");
	ProcMesh->SetCastShadow(false);

	// TODO: ProcMesh attributes, such as bUseAsyncCooking

	ProcMesh->CreateMeshSection_LinearColor(
		VERTICES,
		TArray<FVector>(),
		TArray<int32>(),
		TArray<FVector>(),
		TArray<FVector2D>(),
		TArray<FLinearColor>(),
		TArray<FProcMeshTangent>(),
		true
	);
	ProcMesh->SetMaterial(
		VERTICES,
		ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("/Game/Graphs/Materials/GraphMaterial")).Object
	);
	ProcMesh->CreateMeshSection_LinearColor(
		EDGES,
		TArray<FVector>(),
		TArray<int32>(),
		TArray<FVector>(),
		TArray<FVector2D>(),
		TArray<FLinearColor>(),
		TArray<FProcMeshTangent>(),
		true
	);
	ProcMesh->SetMaterial(
		EDGES,
		ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("/Game/Graphs/Materials/GraphMaterial")).Object
	);

	RootComponent = ProcMesh;
}

void AGraphRenderer::MarkDirty() {
	bool ChangesProvided = false;
	CommandImplementation CmdImpl;
	while (CommandQueue.Dequeue(CmdImpl)) {
		const bool IsSuccess = CmdImpl(Storage);
		if (!ChangesProvided)
			ChangesProvided = IsSuccess;
	}
	if (!ChangesProvided)
		return; // do not redraw meshes if nothing was happened during command execution

	TArray<FVector> Vertices;
	TArray<int32_t> Triangles;
	TArray<FColor> Colors;

	const auto &VerticesStorage = Storage.GetStorage<VertexEntity>().Data;
	const auto &EdgesStorage = Storage.GetStorage<EdgeEntity>().Data;

	if (VerticesStorage.Num() == 0) {
		check(Storage.GetStorage<GraphEntity>().Data.Num() == 0);
		check(Storage.GetStorage<EdgeEntity>().Data.Num() == 0);
		return;
	}

	const size_t VerticesVerticesNum = VertexMeshFactory::MESH_VERTICES_NUM * VerticesStorage.Num();
	const size_t VerticesTrianglesNum = VertexMeshFactory::MESH_TRIANGLES_INDICES_NUM * VerticesStorage.Num();

	const size_t EdgesVerticesNum = EdgeMeshFactory::MESH_VERTICES_NUM * EdgesStorage.Num();
	const size_t EdgesTrianglesNum = EdgeMeshFactory::MESH_TRIANGLES_INDICES_NUM * EdgesStorage.Num();

	check(VerticesTrianglesNum % 3 == 0);
	check(EdgesTrianglesNum % 3 == 0);
	const size_t CollisionDataNum = (VerticesTrianglesNum + EdgesTrianglesNum) / 3;
	CollisionData.Reset(CollisionDataNum); // TODO: update collision data only when calling CreateMeshSection
	TArray<EntityId> EntityCollisionData;

	// Vertices
	{
		Vertices.Reserve(VerticesVerticesNum);
		Triangles.Reserve(VerticesTrianglesNum);
		Colors.Reserve(VerticesVerticesNum);

		for (auto VertexEntIter = VerticesStorage.CreateConstIterator(); VertexEntIter; ++VertexEntIter) {
			VertexMeshFactory::GenerateMesh(
				VertexEntIter->Position,
				VertexEntIter->Selection == EntitySelection::NONE
					? VertexEntIter->Color
					: ColorConsts::BlueColor,
				Vertices, Triangles, Colors
			);

			EntityCollisionData.Init(
				EntityId(VertexEntIter.GetIndex(), EntitySignature::VERTEX),
				VertexMeshFactory::MESH_TRIANGLES_INDICES_NUM / 3
			);
			CollisionData.Append(EntityCollisionData);
		}

		check(Vertices.Num() == VerticesVerticesNum);
		check(Triangles.Num() == VerticesTrianglesNum);
		check(Colors.Num() == VerticesVerticesNum);

		UpdateSection(VERTICES, Vertices, Triangles, Colors);
	}

	// Edges
	if (EdgesStorage.Num() > 0) {
		Vertices.Reset(EdgesVerticesNum);
		Triangles.Reset(EdgesTrianglesNum);
		Colors.Reset(EdgesVerticesNum);

		for (auto EdgeEntIter = EdgesStorage.CreateConstIterator(); EdgeEntIter; ++EdgeEntIter) {
			const auto &FromVertEnt = Storage.GetEntity<VertexEntity>(EdgeEntIter->VerticesIds[0]);
			const auto &ToVertEnt = Storage.GetEntity<VertexEntity>(EdgeEntIter->VerticesIds[1]);
			EdgeMeshFactory::GenerateMesh(
				FromVertEnt.Position, ToVertEnt.Position,
				EdgeEntIter->Selection == EntitySelection::NONE
					? FromVertEnt.Color
					: ColorConsts::BlueColor,
				EdgeEntIter->Selection == EntitySelection::NONE
					? ToVertEnt.Color
					: ColorConsts::BlueColor,
				Vertices, Triangles, Colors
			);

			EntityCollisionData.Init(
				EntityId(EdgeEntIter.GetIndex(), EntitySignature::EDGE),
				EdgeMeshFactory::MESH_TRIANGLES_INDICES_NUM / 3
			);
			CollisionData.Append(EntityCollisionData);
		}

		check(Vertices.Num() == EdgesVerticesNum);
		check(Triangles.Num() == EdgesTrianglesNum);
		check(Colors.Num() == EdgesVerticesNum);

		UpdateSection(EDGES, Vertices, Triangles, Colors);
	}
}

void AGraphRenderer::UpdateSection(
	const int32 SectionIdx,
	const TArray<FVector> &Vertices,
	const TArray<int32_t> &Triangles,
	const TArray<FColor> &Colors
) const {
	if (ProcMesh->GetProcMeshSection(SectionIdx)->ProcIndexBuffer.Num() == Triangles.Num()) {
		ProcMesh->UpdateMeshSection(
			SectionIdx,
			Vertices,
			TArray<FVector>(),
			TArray<FVector2D>(),
			Colors,
			TArray<FProcMeshTangent>()
		);
	}
	else {
		ProcMesh->CreateMeshSection(
			SectionIdx,
			Vertices,
			Triangles,
			TArray<FVector>(),
			TArray<FVector2D>(),
			Colors,
			TArray<FProcMeshTangent>(),
			true
		);
	}
}