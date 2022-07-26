#pragma once

#include "Entities/Entity.h"
#include "GraphProvider.generated.h"

UCLASS()
class GRAPHS_API AGraphProvider final : public AActor {
	GENERATED_BODY()
public:
	AGraphProvider() {
		PrimaryActorTick.bCanEverTick = false;
	}

	template <class Entity>
	Entity &CreateEntity() {
		FActorSpawnParameters s;
		s.Owner = this;
		const auto NewActor = GetWorld()->SpawnActor<Entity>(s);
		Entities.Push(NewActor);
		return *NewActor;
	}

	template <class Command, typename... ArgsType>
	void PushCommand(ArgsType&&... Args) {
		CommandQueue.Enqueue(MakeUnique<Command>(Forward<ArgsType>(Args)...));
	}

	void MarkDirty() {
		while (!CommandQueue.IsEmpty()) {
			CommandQueue.Peek()->Get()->Execute(*this);
			CommandQueue.Pop();
		}
	}

	struct Command {
		Command() = default;
		virtual ~Command() = default;
		virtual void Execute(AGraphProvider &Provider) = 0;
	};
private:
	UPROPERTY()
	TArray<AEntity*> Entities;

	TQueue<TUniquePtr<Command>> CommandQueue;
};
