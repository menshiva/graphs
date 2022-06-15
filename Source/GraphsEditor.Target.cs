using UnrealBuildTool;

public class GraphsEditorTarget : TargetRules {
	public GraphsEditorTarget(TargetInfo target) : base(target) {
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange(new[] { "Graphs" });
	}
}
