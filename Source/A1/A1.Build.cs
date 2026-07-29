// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class A1 : ModuleRules
{
	public A1(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]
		{
			"A1"
		});

		PrivateIncludePaths.Add(ModuleDirectory);

		SetupIrisSupport(Target);

		PublicDependencyModuleNames.AddRange(
			[
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"EnhancedInput",
				"ModularGameplayActors",
				"GameFeatures",
				"CommonCoroutine",
				"CommonGame",
				"NetCore",
				"GameplayTags",
				"GameplayAbilities",
				"GameplayTasks",
				"ModularGameplay",
				"CommonUIExtension",
				"ModelViewViewModel",
				"UMG",
				"CommonUI",
				"Slate",
				"SlateCore",
			]
		);

		// 개발용 디버깅 유틸리티 (FDeveloperPrint 등)
		PrivateDependencyModuleNames.Add("DeveloperTools");
	}
}