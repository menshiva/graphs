using UnrealBuildTool;

public class Graphs : ModuleRules {
	public Graphs(ReadOnlyTargetRules target) : base(target) {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Niagara"
		});
		PrivateDependencyModuleNames.AddRange(new[] { "HeadMountedDisplay" });
		CppStandard = CppStandardVersion.Cpp17;
	}
}
