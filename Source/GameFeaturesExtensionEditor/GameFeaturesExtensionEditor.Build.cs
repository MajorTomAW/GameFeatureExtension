using UnrealBuildTool;

public class GameFeaturesExtensionEditor : ModuleRules
{
    public GameFeaturesExtensionEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Engine",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Slate",
                "SlateCore",
                "InputCore",
                "UnrealEd",
                "AssetDefinition",
                "Projects",
                "GameFeatures",
                "GameFeaturesExtension",
                "SharedSettingsWidgets"
            }
        );
    }
}