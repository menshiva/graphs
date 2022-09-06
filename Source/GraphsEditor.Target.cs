using UnrealBuildTool;

public class GraphsEditorTarget : TargetRules {
	public GraphsEditorTarget(TargetInfo Target) : base(Target) {
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange(new[] { "Graphs" });
	}
}
