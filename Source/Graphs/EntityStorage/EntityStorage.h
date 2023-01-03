#pragma once

#include "Entities/GraphEntity.h"
#include "Entities/VertexEntity.h"
#include "Entities/EdgeEntity.h"

/**
 * Entity system stores and manages all the graph entities (vertices, edges, and graphs).
 * It provides functions for creating and removing entities, and accessing and modifying the stored entities.
 * This class is implemented as a singleton and all its public methods are static.
 *
 * EntityStore uses EntityId to identify and access the stored entities. EntityId is a struct that consists of
 * an index and a signature. The index is used to access the entity in the TSparseArray and the signature is
 * used to differentiate between different types of entities.
 *
 * This class is not intended to be used directly to modify the data, but through its specialized functions provided
 * in the following namespaces:
 * 	- VertexCommands
 * 	- EdgeCommands
 * 	- GraphCommands
 */
class ES {
public:
	/**
	 * A typedef for the implementation of the storage of entities.
	 * @tparam EntityType The type of the entity to be stored.
	 */
	template <typename EntityType>
	using StorageImpl = TSparseArray<EntityType>;

	ES() = default;
	ES(const ES&) = delete;
	ES(ES&&) noexcept = delete;
	ES &operator=(const ES&) = delete;
	ES &operator=(ES&&) noexcept = delete;

	/**
	 * Checks if the given EntityId is a valid identifier for an entity of type EntityType.
	 *
	 * An entity is considered valid if it has the correct signature and its index is within the bounds of the
	 * corresponding storage array.
	 *
	 * @param Id The EntityId to check.
	 * @return True if the EntityId is valid, false otherwise.
	 */
	template <typename EntityType>
	static bool IsValid(const EntityId Id) {
		if (EntityType::Signature != Id.GetSignature() || Id == EntityId::NONE())
			return false;
		return GetInstance().GetStorage<EntityType>().IsValidIndex(Id.GetIndex());
	}

	/**
	 * Retrieves a const reference to the entity identified by the given EntityId.
	 *
	 * @param Id The EntityId of the entity to retrieve.
	 * @return A const reference to the entity data.
	 */
	template <typename EntityType>
	FORCEINLINE static const EntityType &GetEntity(const EntityId Id) {
		check(Id != EntityId::NONE());
		check(EntityType::Signature == Id.GetSignature());
		return GetInstance().GetStorage<EntityType>()[Id.GetIndex()];
	}

	/**
	 * Reserves space for new entities of type EntityType.
	 *
	 * @param NumElementsToAdd The number of entities to reserve space for.
	 */
	template <typename EntityType>
	static void Reserve(size_t NumElementsToAdd) {
		auto &Storage = GetInstance().GetStorage<EntityType>();
		Storage.Reserve(Storage.Num() + NumElementsToAdd);
	}

	/**
	 * Clears all entities of the specified type.
	 * 
	 * @tparam EntityType The type of the entities to clear.
	 */
	template <typename EntityType>
	FORCEINLINE static void Clear() {
		GetInstance().GetStorage<EntityType>().Empty();
	}

	/**
	 * Returns a mutable reference to the entity with the specified EntityId.
	 *
	 * @warning Make sure that the given EntityId is valid.
	 * 
	 * @tparam EntityType The type of the entity.
	 * @param Id The EntityId of the entity.
	 * @return A mutable reference to the entity data.
	 */
	template <typename EntityType>
	FORCEINLINE static EntityType &GetEntityMut(const EntityId Id) {
		check(Id != EntityId::NONE());
		check(EntityType::Signature == Id.GetSignature());
		return GetInstance().GetStorage<EntityType>()[Id.GetIndex()];
	}

	/**
	 * Creates a new entity and returns its EntityId.
	 * 
	 * @tparam EntityType The type of the entity.
	 * @return The EntityId of the new entity.
	 */
	template <typename EntityType>
	FORCEINLINE static EntityId NewEntity() {
		return {
			static_cast<uint32_t>(GetInstance().GetStorage<EntityType>().Emplace()),
			EntityType::Signature
		};
	}

	/**
	 * Removes the entity from the storage.
	 * 
	 * @param Id The EntityId of the entity to remove.
	 */
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

	/**
	 * Returns the storage container for the given entity type.
	 * 
	 * @tparam EntityType The type of entity to get the container for.
	 * @return A reference to the storage container for the given entity type.
	 */
	template <typename EntityType>
	FORCEINLINE StorageImpl<EntityType> &GetStorage() {
		UE_LOG(LogTemp, Fatal, TEXT("Undefined type!"));
		return StorageImpl<EntityType>();
	}

	/**
	 * Specialization of GetStorage for VertexEntity.
	 * 
	 * @return A reference to the storage container for VertexEntity.
	 */
	template <>
	// ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
	FORCEINLINE StorageImpl<VertexEntity> &GetStorage<VertexEntity>() {
		return Vertices;
	}

	/**
	 * Specialization of GetStorage for EdgeEntity.
	 * 
	 * @return A reference to the storage container for EdgeEntity.
	 */
	template <>
	// ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
	FORCEINLINE StorageImpl<EdgeEntity> &GetStorage<EdgeEntity>() {
		return Edges;
	}

	/**
	 * Specialization of GetStorage for GraphEntity.
	 * 
	 * @return A reference to the storage container for GraphEntity.
	 */
	template <>
	// ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
	FORCEINLINE StorageImpl<GraphEntity> &GetStorage<GraphEntity>() {
		return Graphs;
	}

	/** The storage of the VertexEntity type. */
	StorageImpl<VertexEntity> Vertices;
	/** The storage of the EdgeEntity type. */
	StorageImpl<EdgeEntity> Edges;
	/** The storage of the GraphEntity type. */
	StorageImpl<GraphEntity> Graphs;
};
