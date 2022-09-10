#pragma once

namespace Utils {
	template <typename E>
	constexpr static std::underlying_type_t<E> EnumUnderlyingType(E e) noexcept {
		return static_cast<std::underlying_type_t<E>>(e);
	}
}

namespace FileConsts {
	const static FString ExportDirName = "Export\\";
	const static FString ExportFilePrefix = "Graph_";
}

namespace ColorConsts {
	constexpr static FLinearColor VertexDefaultColor = FLinearColor(0.8375f, 0.8375f, 0.8375f);

	constexpr static FLinearColor BlueColor = FLinearColor(0.033345f, 0.066689f, 0.161458f);
	constexpr static FLinearColor GreenColor = FLinearColor(0.033345f, 0.161458f, 0.128114f);
	constexpr static FLinearColor RedColor = FLinearColor(0.161458f, 0.007046f, 0.007046f);
}
