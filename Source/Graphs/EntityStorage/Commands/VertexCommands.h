﻿#pragma once

#include "../EntityStorage.h"
#include "ThirdParty/rapidjson/document.h"
#include "ThirdParty/rapidjson/prettywriter.h"

namespace VertexCommands {
	namespace Mutable {
		EntityId Create(
			EntityId GraphId,
			uint32_t CustomVertexId,
			const FVector &Position,
			const FColor &Color,
			double Value
		);

		void Remove(EntityId VertexId);

		FORCEINLINE void SetHit(const EntityId VertexId, const bool IsHit) {
			ES::GetEntityMut<VertexEntity>(VertexId).IsHit = IsHit;
		}

		FORCEINLINE void SetColor(const EntityId VertexId, const FColor &Color) {
			ES::GetEntityMut<VertexEntity>(VertexId).Color = Color;
		}

		FORCEINLINE void SetOverrideColor(const EntityId VertexId, const FColor &OverrideColor) {
			ES::GetEntityMut<VertexEntity>(VertexId).OverrideColor = OverrideColor;
		}

		FORCEINLINE void SetValue(const EntityId VertexId, const double Value) {
			ES::GetEntityMut<VertexEntity>(VertexId).Value = Value;
		}

		FORCEINLINE void Move(const EntityId VertexId, const FVector &Delta) {
			ES::GetEntityMut<VertexEntity>(VertexId).Position += Delta;
		}

		bool Deserialize(
			const rapidjson::Value &DomVertex,
			EntityId GraphId,
			FString &ErrorMessage
		);
	}

	namespace Const {
		void Serialize(EntityId VertexId, rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer);

		bool AreConnected(EntityId FirstVertexId, EntityId SecondVertexId);
	}
}
