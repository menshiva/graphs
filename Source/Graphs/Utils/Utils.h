#pragma once

namespace Utils {
	/**
	 * Returns a unique integer from the two provided integers using Cantor pairing function.
	 * @link https://en.wikipedia.org/wiki/Pairing_function
	 * 
	 * @param X First integer.
	 * @param Y Second integer.
	 * @return A unique integer calculated using Cantor pairing function.
	 */
	FORCEINLINE constexpr static uint32_t CantorPair(const uint32_t X, const uint32_t Y) {
		return (X + Y) * (X + Y + 1) / 2 + Y;
	}

	/**
	 * Inverse function of Cantor pairing function.
	 * @link https://en.wikipedia.org/wiki/Pairing_function
	 * 
	 * @param Val the value to be unpaired.
	 * @return A pair of values that, when passed to Cantor pairing function, would result in Val.
	 */
	static std::pair<uint32_t, uint32_t> CantorUnpair(const uint32_t Val) {
		// Calculate the value of the upper bound of the pair of integers
		const auto T = static_cast<uint32_t>(floorf(-1.0f + sqrtf(1.0f + 8.0f * Val)) / 2.0f);
		// Calculate and return the pair of integers
		return {
			T * (T + 3) / 2 - Val,
			Val - T * (T + 1) / 2
		};
	}

	/**
	 * Executes the given function on the game thread.
	 *
	 * If the current thread is the game thread, the function is executed immediately.
	 * Otherwise, the function is added to the game thread task queue and will be executed as soon as possible.
	 *
	 * Can be used when an action cannot be performed on a background thread, such as updating the UI.
	 * 
	 * @param InFunction The function to execute on the game thread.
	 */
	template <typename FuncType>
	static void DoOnGameThread(FuncType &&InFunction) {
		if (!IsInGameThread()) {
			FFunctionGraphTask::CreateAndDispatchWhenReady([InFunction(MoveTemp(InFunction))] () mutable {
				InFunction();
			}, TStatId(), nullptr, ENamedThreads::GameThread);
		}
		else InFunction();
	}
}

namespace FileConsts {
	const static FString ExportDirName = "Export\\";
	const static FString ExportFilePrefix = "Graph_";
}

namespace ColorConsts {
	const static FColor VertexDefaultColor = FColor::White;

	const static FColor BlueColor = FLinearColor(0.033345f, 0.066689f, 0.161458f).ToFColor(false);
	const static FColor GreenColor = FLinearColor(0.033345f, 0.161458f, 0.128114f).ToFColor(false);
	const static FColor RedColor = FLinearColor(0.161458f, 0.007046f, 0.007046f).ToFColor(false);

	const static FColor OverrideColorNone = FColor::Transparent;
}

DECLARE_STATS_GROUP(TEXT("GRAPHS_PERF"), STATGROUP_GRAPHS_PERF, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("GRAPHS_PERF_COMMANDS"), STATGROUP_GRAPHS_PERF_COMMANDS, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("GRAPHS_PERF_GRAPHS_RENDERERS"), GRAPHS_PERF_GRAPHS_RENDERERS, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("GRAPHS_PERF_GRAPH_CHUNK_RENDERER"), GRAPHS_PERF_GRAPH_CHUNK_RENDERER, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("GRAPHS_PERF_VERTICES_RENDERER"), GRAPHS_PERF_VERTICES_RENDERER, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("GRAPHS_PERF_EDGES_RENDERER"), GRAPHS_PERF_EDGES_RENDERER, STATCAT_Advanced);
