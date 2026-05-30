using UnrealBuildTool;

public class DeveloperTools : ModuleRules
{
    public DeveloperTools(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.Add(ModuleDirectory);

        PublicDependencyModuleNames.AddRange(["Core"]);
        PrivateDependencyModuleNames.AddRange(["CoreUObject", "Engine"]);
    }
}
