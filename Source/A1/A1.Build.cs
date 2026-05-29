// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class A1 : ModuleRules {
  public A1(ReadOnlyTargetRules Target) : base(Target) {
    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

    PrivateIncludePaths.Add(ModuleDirectory);

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
      //"NetCore",
      //"GameplayTags",
      //"GameplayAbilities",
      //"GameplayTasks",
      //"ModularGameplay",
      //"CommonUIExtension",
      //"ModelViewViewModel",
      //"UMG",
      //"CommonUI",
      //"Slate",
      //"SlateCore",
      ]
      );
  }
}
