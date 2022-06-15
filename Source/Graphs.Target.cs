using UnrealBuildTool;

public class GraphsTarget : TargetRules {
	public GraphsTarget(TargetInfo target) : base(target) {
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange(new[] { "Graphs" });
	}
}
