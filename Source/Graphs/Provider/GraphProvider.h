#pragma once

#include "Entities/Entities.h"
#include "GraphProvider.generated.h"

UCLASS()
class GRAPHS_API AGraphProvider final : public AActor {
	GENERATED_BODY()
public:
	AGraphProvider();

	// TODO: make private and add friend class Command
	template <class T>
	T &CreateComponent() {
		FActorSpawnParameters s;
		s.Owner = this;
		const auto NewActor = GetWorld()->SpawnActor<T>(s);
		Entities.Add(NewActor->GetId(), NewActor);
		check(NewActor->IsValid());
		return *NewActor;
	}
private:
	UPROPERTY()
	TMap<uint32, AEntity*> Entities;
};
