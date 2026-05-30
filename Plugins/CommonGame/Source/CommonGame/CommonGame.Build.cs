// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CommonGame : ModuleRules
{
    public CommonGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.Add(ModuleDirectory);

        PublicDependencyModuleNames.AddRange([
            "Core",
            "CoreUObject",
            "Engine",
            "NetCore",
            "ModularGameplay",
            "GameFeatures",
            "ModularGameplayActors",
            "EnhancedInput",
            "GameplayTags",
            "GameplayCameras",
            "GameplayAbilities",
            "GameplayTasks",
            "AnimGraphRuntime",
            "IrisCore",
            "CommonCoroutine"]);

        SetupIrisSupport(Target);

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(["DeveloperTools", "AssetRegistry"]);
        }
    }
}
