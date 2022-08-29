using UnrealBuildTool;

public class GraphsTarget : TargetRules {
	public GraphsTarget(TargetInfo Target) : base(Target) {
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange(new[] { "Graphs" });
	}
}
