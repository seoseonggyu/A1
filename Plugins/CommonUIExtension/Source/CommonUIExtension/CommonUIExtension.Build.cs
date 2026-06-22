// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CommonUIExtension : ModuleRules
{
	public CommonUIExtension(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange([
			"Core",
			"CommonUI",
			"ModelViewViewModel",
			"UMG",
			"Slate",
			"SlateCore",
			"GameplayTags"
		]);

		PrivateDependencyModuleNames.AddRange([
			"CoreUObject",
			"Engine",
			"InputCore",
			"CommonGame",
			"CommonCoroutine",
			"ModularGameplay",
			"GameFeatures"
		]);
	}
}
