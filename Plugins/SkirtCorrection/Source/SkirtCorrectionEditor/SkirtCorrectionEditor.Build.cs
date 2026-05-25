using UnrealBuildTool;

public class SkirtCorrectionEditor : ModuleRules
{
    public SkirtCorrectionEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "AnimGraph",
            "BlueprintGraph",
            "SkirtCorrectionRuntime"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "UnrealEd"
        });
    }
}