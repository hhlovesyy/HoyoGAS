// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class HoyoGas : ModuleRules
{
	public HoyoGas(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]
		{
			Path.Combine(ModuleDirectory, "GameplayDemo/Public"),
			Path.Combine(ModuleDirectory, "CharacterPanel/Public"),
			Path.Combine(ModuleDirectory, "CharacterGrowth/Public"),
			Path.Combine(ModuleDirectory, "UIFramework/Public"),
			Path.Combine(ModuleDirectory, "UIViewModels/Public"),
			Path.Combine(ModuleDirectory, "UIGenerated/UIFramework/Public"),
			Path.Combine(ModuleDirectory, "UIGenerated/UIViewModels/Public"),
			Path.Combine(ModuleDirectory, "OrigamiBird/Public"),
			Path.Combine(ModuleDirectory, "SurvivorArena/Public"),
		});

		PrivateIncludePaths.AddRange(new string[]
		{
			ModuleDirectory,
			Path.Combine(ModuleDirectory, "GameplayDemo/Private"),
			Path.Combine(ModuleDirectory, "CharacterPanel/Private"),
			Path.Combine(ModuleDirectory, "CharacterGrowth/Private"),
			Path.Combine(ModuleDirectory, "UIFramework/Private"),
			Path.Combine(ModuleDirectory, "UIViewModels/Private"),
			Path.Combine(ModuleDirectory, "UIGenerated/UIFramework/Private"),
			Path.Combine(ModuleDirectory, "UIGenerated/UIViewModels/Private"),
			Path.Combine(ModuleDirectory, "OrigamiBird/Private"),
			Path.Combine(ModuleDirectory, "SurvivorArena/Private"),
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"DeveloperSettings",
			"Engine",
			"CommonInput",
			"CommonUI",
			"InputCore",
			"EnhancedInput",
			"FieldNotification",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"HTTP",
			"Json",
			"JsonUtilities",
			"ModelViewViewModel",
			"UMG",
			"Slate",
			"SlateCore"
		});
		
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"AssetRegistry",
				"AssetTools",
				"Blutility",
				"IKRig",
				"IKRigEditor",
				"UnrealEd"
			});
		}
	}
}
