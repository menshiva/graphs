#pragma once

namespace FileConsts {
	const static FString ExportDirName = "Export\\";
	const static FString ExportFilePrefix = "Graph_";
}

namespace ColorConsts {
	constexpr static FLinearColor GraphDefaultColor = FLinearColor(0.4375f, 0.4375f, 0.4375f);

	constexpr static FLinearColor BlueColor = FLinearColor(0.033345f, 0.066689f, 0.161458f);
	constexpr static FLinearColor GreenColor = FLinearColor(0.033345f, 0.161458f, 0.128114f);
	constexpr static FLinearColor RedColor = FLinearColor(0.161458f, 0.007046f, 0.007046f);
}

DECLARE_STATS_GROUP(TEXT("GRAPHS_PRERF"), STATGROUP_GRAPHS_PERF, STATCAT_Advanced); 
DECLARE_STATS_GROUP(TEXT("GRAPHS_PRERF_COMMANDS"), STATGROUP_GRAPHS_PERF_COMMANDS, STATCAT_Advanced); 
