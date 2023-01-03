#pragma once

#include "Entities/GraphEntity.h"
#include "Entities/VertexEntity.h"
#include "Entities/EdgeEntity.h"

/**
 * Entity system responsible for creating, modifying, and deleting entities.
 *
 * It stores three types of entities: vertices, edges, and graphs.
 * Each entity has a unique EntityId, which consists of an index and a signature.
 * The index is used to access the entity in the corresponding storage,
 * while the signature is used to determine the type of the entity (in which storage entity is contained).
 *
 * This class is implemented as a singleton and all its public methods are static.
 */
class ES {
public:
	template <typename EntityType>
	using StorageImpl = TSparseArray<EntityType>;

	ES() = default;
	ES(const ES&) = delete;
	ES(ES&&) noexcept = delete;
	ES &operator=(const ES&) = delete;
	ES &operator=(ES&&) noexcept = delete;

	/** Checks that the storage of EntityType contains an entity at Id. */
	template <typename EntityType>
	static bool IsValid(const EntityId Id) {
		if (EntityType::Signature != Id.GetSignature() || Id == EntityId::NONE())
			return false;
		return GetInstance().GetStorage<EntityType>().IsValidIndex(Id.GetIndex());
	}

	/** Returns const data of an entity at Id from the storage of EntityType. */
	template <typename EntityType>
	FORCEINLINE static const EntityType &GetEntity(const EntityId Id) {
		check(Id != EntityId::NONE());
		check(EntityType::Signature == Id.GetSignature());
		return GetInstance().GetStorage<EntityType>()[Id.GetIndex()];
	}

	/** Preallocates memory for the storage of EntityType. */
	template <typename EntityType>
	static void Reserve(size_t NumElementsToAdd) {
		auto &Storage = GetInstance().GetStorage<EntityType>();
		Storage.Reserve(Storage.Num() + NumElementsToAdd);
	}

	/** Removes all elements from the storage of EntityType. */
	template <typename EntityType>
	FORCEINLINE static void Clear() {
		GetInstance().GetStorage<EntityType>().Empty();
	}

	/** Returns mutable data of an entity at Id from the storage of EntityType. */
	template <typename EntityType>
	FORCEINLINE static EntityType &GetEntityMut(const EntityId Id) {
		check(Id != EntityId::NONE());
		check(EntityType::Signature == Id.GetSignature());
		return GetInstance().GetStorage<EntityType>()[Id.GetIndex()];
	}

	/** Creates a new entity of EntityType in the first free position of the corresponding storage. */
	template <typename EntityType>
	FORCEINLINE static EntityId NewEntity() {
		return {
			static_cast<uint32_t>(GetInstance().GetStorage<EntityType>().Emplace()),
			EntityType::Signature
		};
	}

	/** Removes an entity at Id from the storage of EntityType. */
	template <typename EntityType>
	FORCEINLINE static void RemoveEntity(const EntityId Id) {
		check(Id != EntityId::NONE());
		check(EntityType::Signature == Id.GetSignature());
		GetInstance().GetStorage<EntityType>().RemoveAt(Id.GetIndex());
	}
private:
	static ES &GetInstance() {
		static ES Singleton;
		return Singleton;
	}

	template <typename EntityType>
	FORCEINLINE StorageImpl<EntityType> &GetStorage() {
		UE_LOG(LogTemp, Fatal, TEXT("Undefined type!"));
		return StorageImpl<EntityType>();
	}

	template <>
	// ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
	FORCEINLINE StorageImpl<VertexEntity> &GetStorage<VertexEntity>() {
		return Vertices;
	}

	template <>
	// ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
	FORCEINLINE StorageImpl<EdgeEntity> &GetStorage<EdgeEntity>() {
		return Edges;
	}

	template <>
	// ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
	FORCEINLINE StorageImpl<GraphEntity> &GetStorage<GraphEntity>() {
		return Graphs;
	}

	StorageImpl<VertexEntity> Vertices;
	StorageImpl<EdgeEntity> Edges;
	StorageImpl<GraphEntity> Graphs;
};
