﻿#pragma once

#include "EntityStorage/EntityStorage.h"
#include "GraphRenderer.generated.h"

UCLASS()
class GRAPHS_API AGraphRenderer final : public AActor {
	GENERATED_BODY()
public:
	using CommandImplementation = TFunction<void(EntityStorage &Storage)>;

	class Command {
	public:
		explicit Command(CommandImplementation &&Impl) : Implementation(MoveTemp(Impl)) {}
		virtual ~Command() = default;
	protected:
		CommandImplementation Implementation;
		friend AGraphRenderer;
	};

	AGraphRenderer();

	FORCEINLINE const EntityStorage &GetEntityStorage() const {
		return Storage;
	}

	FORCEINLINE void PushCommand(Command &&Cmd) {
		CommandQueue.Enqueue(MoveTemp(Cmd.Implementation));
	}

	void MarkDirty();

	void UpdateSection(
		int32 SectionIdx,
		const TArray<FVector> &Vertices,
		const TArray<int32_t> &Triangles,
		const TArray<FLinearColor> &Colors
	) const;
private:
	UPROPERTY()
	class UProceduralMeshComponent *ProcMesh;

	EntityStorage Storage;
	TQueue<CommandImplementation> CommandQueue;
};
