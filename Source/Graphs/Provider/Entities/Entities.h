#pragma once

#include "Engine/StaticMeshActor.h"
#include "Entities.generated.h"

enum class EntityType : uint8_t {
	GRAPH = 0,
	NODE,
	CONNECTION
};

constexpr static uint32 ENTITY_NONE = static_cast<uint32>(-1);

UCLASS(Abstract)
class GRAPHS_API AEntity : public AStaticMeshActor {
	GENERATED_BODY()
public:
	AEntity() = default;
	AEntity(const TCHAR *MeshAssetName, const TCHAR *MeshMaterialAssetName, EntityType EntType);

	FORCEINLINE uint32 GetId() const { return GetUniqueID(); }
	FORCEINLINE EntityType GetType() const { return Type; }
private:
	EntityType Type;
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
