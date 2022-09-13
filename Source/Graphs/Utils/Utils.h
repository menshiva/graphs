#pragma once

namespace Utils {
	template <typename E>
	constexpr static std::underlying_type_t<E> EnumUnderlyingType(E e) noexcept {
		return static_cast<std::underlying_type_t<E>>(e);
	}

	constexpr static uint32_t CantorPair(const uint32_t X, const uint32_t Y) {
		return (X + Y) * (X + Y + 1) / 2 + Y;
	}

	static std::pair<uint32_t, uint32_t> CantorUnpair(const uint32_t Val) {
		const auto T = static_cast<uint32_t>(floorf(-1.0f + sqrtf(1.0f + 8.0f * Val)) / 2.0f);
		return {
			T * (T + 3) / 2 - Val,
			Val - T * (T + 1) / 2
		};
	}
}

namespace FileConsts {
	const static FString ExportDirName = "Export\\";
	const static FString ExportFilePrefix = "Graph_";
}

namespace ColorConsts {
    // TODO
	// const static FColor VertexDefaultColor = FLinearColor(0.4375f, 0.4375f, 0.4375f).ToFColor(false);

	const static FColor BlueColor = FLinearColor(0.033345f, 0.066689f, 0.161458f).ToFColor(false);
	const static FColor GreenColor = FLinearColor(0.033345f, 0.161458f, 0.128114f).ToFColor(false);
	const static FColor RedColor = FLinearColor(0.161458f, 0.007046f, 0.007046f).ToFColor(false);

	const static FColor OverrideColorNone = FColor::Transparent;
}
