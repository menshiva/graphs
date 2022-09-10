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
			"InputCore",
			"ProceduralMeshComponent"
		});
		PrivateDependencyModuleNames.AddRange(new[] {
			"HeadMountedDisplay",
			"SlateCore"
		});
		PublicIncludePaths.Add(EngineThirdPartyPath);

		CppStandard = CppStandardVersion.Latest;
		bUseRTTI = true;
	}
}
