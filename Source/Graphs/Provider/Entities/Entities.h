#pragma once

#include "Engine/StaticMeshActor.h"
#include "Entities.generated.h"

enum class EntityType : uint8_t {
	GRAPH = 0,
	NODE,
	CONNECTION,
	UNDENTIFIED
};

UCLASS(Abstract)
class GRAPHS_API AEntity : public AStaticMeshActor {
	GENERATED_BODY()
public:
	AEntity() = default;
	explicit AEntity(const TCHAR *MeshAssetName, const TCHAR *MeshMaterialAssetName, EntityType EntType);

	FORCEINLINE uint32 GetId() const;
	FORCEINLINE EntityType GetType() const;
	FORCEINLINE bool IsValid() const;
private:
	EntityType Type = EntityType::UNDENTIFIED;
};

UCLASS()
class GRAPHS_API ANodeEntity final : public AEntity {
	GENERATED_BODY()
public:
	ANodeEntity() : AEntity(
		TEXT("/Engine/BasicShapes/Sphere"),
		TEXT("/Game/Graphs/Materials/GraphMaterial"),
		EntityType::NODE
	) {}
};