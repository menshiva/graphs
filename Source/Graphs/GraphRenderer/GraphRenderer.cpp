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

	TArray<FVector> Vertices;
	TArray<int32_t> Triangles;
	TArray<FLinearColor> Colors;

	// Vertices
	const auto &VerticesStorage = Storage.GetStorage<VertexEntity>().Data;
	if (VerticesStorage.Num() == 0) {
		check(Storage.GetStorage<GraphEntity>().Data.Num() == 0);
		check(Storage.GetStorage<EdgeEntity>().Data.Num() == 0);
		return;
	}
	{
		const size_t VerticesNum = VertexMeshFactory::MESH_VERTICES_NUM * VerticesStorage.Num();
		const size_t TrianglesNum = VertexMeshFactory::MESH_TRIANGLES_INDICES_NUM * VerticesStorage.Num();

		Vertices.Reserve(VerticesNum);
		Triangles.Reserve(TrianglesNum);
		Colors.Reserve(VerticesNum);

		for (const auto &VertexEnt : VerticesStorage) {
			VertexMeshFactory::GenerateMesh(
				VertexEnt.Position, VertexEnt.Color,
				Vertices, Triangles, Colors
			);
		}

		check(Vertices.Num() == VerticesNum);
		check(Triangles.Num() == TrianglesNum);
		check(Colors.Num() == VerticesNum);

		UpdateSection(VERTICES, Vertices, Triangles, Colors);
	}

	// Edges
	const auto &EdgesStorage = Storage.GetStorage<EdgeEntity>().Data;
	if (EdgesStorage.Num() > 0) {
		// clear arrays but not memory allocations
		Vertices.Reset(Vertices.Max());
		Triangles.Reset(Triangles.Max());
		Colors.Reset(Colors.Max());

		// TODO
	}

	// TODO: remove
	/*TArray<FVector> vertices;
	vertices.Add(FVector(0, 0, 0));
	vertices.Add(FVector(0, 100, 0));
	vertices.Add(FVector(0, 50, 100));

	TArray<int32> triangles;
	triangles.Add(0);
	triangles.Add(1);
	triangles.Add(2);

	TArray<FVector2D> UV0;
	UV0.Add(FVector2D(0.0f, 0.0f));
	UV0.Add(FVector2D(1.0f, 0.0f));
	UV0.Add(FVector2D(0.0f, 1.0f));

	TArray<FLinearColor> vertexColors;
	vertexColors.Add(FLinearColor::Blue);
	vertexColors.Add(FLinearColor::Green);
	vertexColors.Add(FLinearColor::Red);

	ProcMesh->CreateMeshSection_LinearColor(EDGES, vertices, triangles, TArray<FVector>(), UV0, vertexColors, TArray<FProcMeshTangent>(), true);*/
}

void AGraphRenderer::UpdateSection(
	const int32 SectionIdx,
	const TArray<FVector> &Vertices,
	const TArray<int32_t> &Triangles,
	const TArray<FLinearColor> &Colors
) const {
	if (ProcMesh->GetProcMeshSection(SectionIdx)->ProcIndexBuffer.Num() == Triangles.Num()) {
		ProcMesh->UpdateMeshSection_LinearColor(
			SectionIdx,
			Vertices,
			TArray<FVector>(),
			TArray<FVector2D>(),
			Colors,
			TArray<FProcMeshTangent>()
		);
	}
	else {
		ProcMesh->CreateMeshSection_LinearColor(
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
