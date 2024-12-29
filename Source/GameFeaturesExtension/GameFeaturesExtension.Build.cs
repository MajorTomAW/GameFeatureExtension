// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameFeaturesExtension : ModuleRules
{
	public GameFeaturesExtension(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"GameFeatures",
			"GameplayAbilities",
			"GameplayTags",
			"EnhancedInput",
			"CommonUI",
			"UIExtension",
			"UMG",
		});


		PrivateDependencyModuleNames.AddRange(new[]
		{
			"CoreUObject",
			"Engine",
			"CommonGame",
			"ModularGameplay",
		});
	}
}
