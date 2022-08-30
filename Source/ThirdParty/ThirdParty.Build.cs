using System.IO;
using UnrealBuildTool;

public class ThirdParty : ModuleRules {
	public ThirdParty(ReadOnlyTargetRules Target) : base(Target) {
		Type = ModuleType.External;
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "nlohmann"));
	}
}
