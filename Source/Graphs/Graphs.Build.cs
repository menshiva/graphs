using UnrealBuildTool;

public class Graphs : ModuleRules {
	public Graphs(ReadOnlyTargetRules target) : base(target) {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Niagara",
			"UMG"
		});
		PrivateDependencyModuleNames.AddRange(new[] { "HeadMountedDisplay", "Slate", "SlateCore" });
		CppStandard = CppStandardVersion.Cpp17;
	}
}
