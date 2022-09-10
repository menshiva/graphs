#include "GraphRenderer.h"
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
	CommandImplementation CmdImpl;
	while (CommandQueue.Dequeue(CmdImpl)) {
		CmdImpl(Storage);
	}

	// Vertices
	{
		const auto &VerticesStorage = Storage.GetStorage<VertexEntity>();
		const size_t VerticesNum = VertexMeshFactory::MESH_VERTICES_NUM * VerticesStorage.Data.Num();
		const size_t TrianglesNum = VertexMeshFactory::MESH_TRIANGLES_INDICES_NUM * VerticesStorage.Data.Num();

		TArray<FVector> Vertices;
		Vertices.Reserve(VerticesNum);
		TArray<int32_t> Triangles;
		Triangles.Reserve(TrianglesNum);
		TArray<FLinearColor> Colors;
		Colors.Reserve(VerticesNum);

		for (const auto &VertexEnt : VerticesStorage.Data) {
			VertexMeshFactory::GenerateMesh(
				VertexEnt.Position, VertexEnt.Color,
				Vertices, Triangles, Colors
			);
		}

		check(Vertices.Num() == VerticesNum);
		check(Triangles.Num() == TrianglesNum);
		check(Colors.Num() == VerticesNum);

		if (ProcMesh->GetProcMeshSection(VERTICES)->ProcIndexBuffer.Num() == TrianglesNum) {
			ProcMesh->UpdateMeshSection_LinearColor(
				VERTICES,
				Vertices,
				TArray<FVector>(),
				TArray<FVector2D>(),
				Colors,
				TArray<FProcMeshTangent>()
			);
		}
		else {
			ProcMesh->CreateMeshSection_LinearColor(
				VERTICES,
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
}
