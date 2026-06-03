// Copyright Epic Games, Inc. All Rights Reserved.

#include "Action/GameFeatureAction_AddActorExtension.h"
#include "TimerManager.h"
#include "Extension/ActorExtensionWorldSubsystem.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddActorExtension)

void UGameFeatureAction_AddActorExtension::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	// 모든 World에서 Extension 해제
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (UWorld* World = WorldContext.World())
		{
			UnregisterExtension(World);
		}
	}
}

void UGameFeatureAction_AddActorExtension::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	if (!World)
	{
		return;
	}

	RegisterExtension(World);
}

void UGameFeatureAction_AddActorExtension::RegisterExtension(UWorld* World)
{
	if (!TargetClass)
	{
		return;
	}

	// OnGameInstanceWorldChanged 델리게이트는 World 전환 시점에 호출되는데,
	// 이 시점에는 WorldSubsystem 초기화가 완료되지 않았을 수 있습니다.
	// Subsystem이 없으면 다음 프레임에 재시도합니다.
	UActorExtensionWorldSubsystem* Subsystem = World->GetSubsystem<UActorExtensionWorldSubsystem>();
	if (!Subsystem)
	{
		TWeakObjectPtr WeakWorld = World;
		TWeakObjectPtr WeakThis = this;
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([WeakThis, WeakWorld]()
		{
			if (ThisClass* This = WeakThis.Get())
			{
				if (UWorld* World = WeakWorld.Get())
				{
					This->RegisterExtension(World);
				}
			}
		}));
		return;
	}

	Subsystem->RegisterExtensionForClass(TargetClass.Get(), Extension, bAddToLocallyControlled, bAddToSimulatedProxy);
}

void UGameFeatureAction_AddActorExtension::UnregisterExtension(UWorld* World)
{
	if (!TargetClass)
	{
		return;
	}

	UActorExtensionWorldSubsystem* Subsystem = World->GetSubsystem<UActorExtensionWorldSubsystem>();
	if (!Subsystem)
	{
		return;
	}

	Subsystem->UnregisterExtensionsForClass(TargetClass.Get());
}
