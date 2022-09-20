#pragma once

#include "Graphs/Utils/Utils.h"

enum EntitySignature : uint8_t {
	VERTEX = 0,
	EDGE,
	GRAPH,
	SIZE
};

struct EntityId {
	static EntityId NONE() {
		static EntityId NoneId;
		return NoneId;
	}

	EntityId() : EntityId(-1, SIZE) {}
	EntityId(const uint32_t Index, const EntitySignature Signature) : Index(Index), Signature(Signature) {}

	bool operator==(const EntityId &OtherId) const {
		return Signature == OtherId.Signature && Index == OtherId.Index;
	}

	bool operator!=(const EntityId &OtherId) const {
		return !(*this == OtherId);
	}

	FORCEINLINE static uint32 Hash(const EntityId &Id) {
		return Utils::CantorPair(Id.Index, Id.Signature);
	}

	FORCEINLINE static EntityId Unhash(const uint32 Hash) {
		const auto P = Utils::CantorUnpair(Hash);
		return EntityId(P.first, static_cast<EntitySignature>(P.second));
	}
private:
	uint32_t Index;
	EntitySignature Signature;

	friend class UTool;
	friend class ES;
};

FORCEINLINE uint32 GetTypeHash(const EntityId &Id) {
	return EntityId::Hash(Id);
}
