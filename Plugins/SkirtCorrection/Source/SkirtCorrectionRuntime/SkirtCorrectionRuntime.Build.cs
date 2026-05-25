using UnrealBuildTool;

public class SkirtCorrectionRuntime : ModuleRules
{
    public SkirtCorrectionRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "AnimGraphRuntime"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
        });
    }
}