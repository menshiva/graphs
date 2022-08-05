#pragma once

#include "Entities/Entity.h"
#include "GraphProvider.generated.h"

UCLASS()
class GRAPHS_API AGraphProvider final : public AActor {
	GENERATED_BODY()
public:
	AGraphProvider() {
		PrimaryActorTick.bCanEverTick = false;

		VertexMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Engine/BasicShapes/Sphere")).Object;
		VertexMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("/Game/Graphs/Materials/GraphMaterial")).Object;
	}

	FORCEINLINE UStaticMesh *GetVertexMesh() const { return VertexMesh; }
	FORCEINLINE UMaterial *GetVertexMaterial() const { return VertexMaterial; }

	FORCEINLINE bool IsEntityValid(const EntityId Id) const { return Entities.Contains(Id); }
	FORCEINLINE const Entity *GetConstEntity(const EntityId Id) const { return Entities.Find(Id)->Get(); }

	class Command {
	public:
		explicit Command(TFunction<void(AGraphProvider &Provider)> &&Impl) : Implementation(MoveTemp(Impl)) {}
		virtual ~Command() = default;
	protected:
		template <class Ent>
		static Ent *CreateEntity(AGraphProvider &Provider) {
			const auto NewEntity = new Ent(Provider);
			const auto ElementId = Provider.Entities.Emplace(NewEntity);
			const auto Actor = NewEntity->GetActor();
			Actor->AttachToActor(&Provider, FAttachmentTransformRules::KeepRelativeTransform);
			Provider.Actors.Add(Actor);
			return NewEntity;
		}

		static Entity *GetMutEntity(const AGraphProvider &Provider, const EntityId Id) {
			return Provider.Entities.Find(Id)->Get();
		}

		static void RemoveEntity(AGraphProvider &Provider, const EntityId Id) {
			const auto FoundEntity = GetMutEntity(Provider, Id);
			const auto EntityActor = FoundEntity->GetActor();
			Provider.Actors.Remove(EntityActor);
			Provider.Entities.Remove(Id);
			Provider.GetWorld()->DestroyActor(EntityActor);
		}
	private:
		TFunction<void(AGraphProvider &Provider)> Implementation;
		friend AGraphProvider;
	};

	template <class T, typename... ArgsType>
	void ExecuteCommand(ArgsType&&... Args) {
		T Cmd(Forward<ArgsType>(Args)...);
		Cmd.Implementation(*this);
	}
private:
	UPROPERTY()
	UStaticMesh *VertexMesh;

	UPROPERTY()
	UMaterial *VertexMaterial;

	UPROPERTY()
	TSet<AActor*> Actors;

	TSet<TUniquePtr<Entity>, EntityKeyFuncs> Entities;
};
