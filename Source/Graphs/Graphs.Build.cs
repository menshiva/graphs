using System.IO;
using UnrealBuildTool;

public class Graphs : ModuleRules {
	private string EnginePath {
		get { return Path.GetFullPath(Target.RelativeEnginePath); }
	}

	private string EngineThirdPartyPath {
		get { return Path.GetFullPath(Path.Combine(EnginePath, "Source/ThirdParty/")); }
	}

	public Graphs(ReadOnlyTargetRules Target) : base(Target) {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Niagara",
			"UMG",
			"DesktopPlatform"
		});
		PrivateDependencyModuleNames.AddRange(new[] { "HeadMountedDisplay", "Slate", "SlateCore" });
		PublicIncludePaths.Add(EngineThirdPartyPath);

		CppStandard = CppStandardVersion.Cpp17;
		bUseRTTI = true;
	}
}
