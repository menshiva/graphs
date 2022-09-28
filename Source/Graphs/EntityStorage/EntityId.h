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

	FORCEINLINE static uint32 Hash(const EntityId Id) {
		return Utils::CantorPair(Id.Index, Id.Signature);
	}

	static EntityId Unhash(const uint32 Hash) {
		const auto P = Utils::CantorUnpair(Hash);
		return EntityId(P.first, static_cast<EntitySignature>(P.second));
	}
private:
	uint32_t Index;
	EntitySignature Signature;
};

FORCEINLINE uint32 GetTypeHash(const EntityId Id) {
	return EntityId::Hash(Id);
}
