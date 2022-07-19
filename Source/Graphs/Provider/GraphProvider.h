#pragma once

#include "Entities/Entities.h"
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
		Entities.Add(NewActor->GetId(), NewActor);
		return *NewActor;
	}

	template <class Entity>
	Entity *GetEntity(const uint32 EntityId) {
		return Cast<Entity>(Entities.FindChecked(EntityId));
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
	TMap<uint32, AEntity*> Entities;

	TQueue<TUniquePtr<Command>> CommandQueue;
};
