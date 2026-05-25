using UnrealBuildTool;

public class JsonWidgetBlueprintImporterEditor : ModuleRules
{
    public JsonWidgetBlueprintImporterEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "CommonUI",
            "Core",
            "CoreUObject",
            "FieldNotification",
            "Engine",
            "InputCore",
            "Json",
            "ModelViewViewModel",
            "ModelViewViewModelBlueprint",
            "ModelViewViewModelEditor",
            "Slate",
            "SlateCore",
            "UMG"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "ApplicationCore",
            "AssetTools",
            "CoreUObject",
            "DesktopPlatform",
            "EditorFramework",
            "LevelEditor",
            "Projects",
            "Slate",
            "SlateCore",
            "ToolMenus",
            "UMGEditor",
            "UnrealEd"
        });
    }
}
