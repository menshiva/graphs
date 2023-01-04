#pragma once

#include "Graphs/Utils/Utils.h"

/** Enum used to identify a type of entity. */
enum EntitySignature : uint8_t {
	VERTEX = 0,
	EDGE,
	GRAPH,
	/**
	 * Represents the number of possible entity types, and is also considered an invalid type.
	 * It can be used, for example, to check if an `EntityId` is valid or not.
	 */
	SIZE
};

/**
 * Struct representing the identifier of a specific entity in EntityStorage.
 *
 * It consists of a pair of a 32-bit unsigned integer `Index` and an `EntitySignature` value.
 * The `Index` represents the position of the entity in the sparse array in EntityStorage.
 * The `Signature` is an enum value of type EntitySignature that represents the type of the entity (VERTEX, EDGE, or GRAPH).
 */
struct EntityId {
	/**
	 * Returns a special value of EntityId that represents an invalid or non-existent entity.
	 * @return EntityId with Index set to -1 and Signature set to SIZE.
	 */
	static EntityId NONE() {
		static EntityId NoneId(-1, SIZE);
		return NoneId;
	}

	EntityId(const uint32_t Index, const EntitySignature Signature) : Index(Index), Signature(Signature) {}

	bool operator<(const EntityId OtherId) const {
		if (Index < OtherId.Index)
			return true;
		if (OtherId.Index < Index)
			return false;
		return Signature < OtherId.Signature;
	}

	bool operator==(const EntityId OtherId) const {
		return Signature == OtherId.Signature && Index == OtherId.Index;
	}

	bool operator!=(const EntityId OtherId) const {
		return !(*this == OtherId);
	}

	FORCEINLINE uint32_t GetIndex() const {
		return Index;
	}

	FORCEINLINE EntitySignature GetSignature() const {
		return Signature;
	}

	/**
	 * Used to generate a hash value for a given EntityId.
	 *
	 * The hash is computed using the Cantor pairing function, which combines the Index and Signature values of the
	 * EntityId into a single integer.
	 */
	FORCEINLINE static uint32 Hash(const EntityId Id) {
		return Utils::CantorPair(Id.Index, Id.Signature);
	}

	/**
	 * Used to compute the original EntityId that was used to generate a given hash.
	 *
	 * It does this by using the Cantor unpairing function to separate the Index and Signature values from the hash,
	 * and then creates a new EntityId using those values.
	 */
	static EntityId Unhash(const uint32 Hash) {
		const auto [Index, Signature] = Utils::CantorUnpair(Hash);
		return EntityId(Index, static_cast<EntitySignature>(Signature));
	}
private:
	uint32_t Index;
	EntitySignature Signature;
};

/** Function for UE to be able to use EntityId in TSet. */
FORCEINLINE uint32 GetTypeHash(const EntityId Id) {
	return EntityId::Hash(Id);
}
