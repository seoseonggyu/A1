// Copyright Epic Games, Inc. All Rights Reserved.

#include "Action/GameFeatureAction_AddAbilities.h"
#include "AbilitySystem/CommonAbilitySystemComponent.h"
#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "AbilitySystemGlobals.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFeaturesSubsystem.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddAbilities)

DEFINE_LOG_CATEGORY(GameFeatureAction_AddAbilitiesLog);

UGameFeatureAction_AddAbilities::UGameFeatureAction_AddAbilities()
{
	// Ability 부여는 서버에서만 실행
	bClientAction = false;
	bServerAction = true;
}

void UGameFeatureAction_AddAbilities::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	FGameFeatureStateChangeContext StateChangeContext(Context);

	// 해당 Context의 핸들만 제거
	if (FAbilityGrantedHandles* Handles = ContextHandles.Find(StateChangeContext))
	{
		// GFCM 핸들 해제
		Handles->ExtensionRequestHandle.Reset();

		// 부여된 Ability 제거
		for (auto& Pair : Handles->AbilitySpecHandles)
		{
			if (AActor* Actor = Pair.Key.Get())
			{
				if (UCommonAbilitySystemComponent* ASC = GetAbilitySystemComponent(Actor))
				{
					for (const FGameplayAbilitySpecHandle& Handle : Pair.Value)
					{
						ASC->ClearAbility(Handle);
					}
				}
			}
		}

		ContextHandles.Remove(StateChangeContext);
	}
}

void UGameFeatureAction_AddAbilities::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	UGameFrameworkComponentManager* GFCM = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(WorldContext.OwningGameInstance);
	if (!GFCM)
	{
		return;
	}

	if (TargetClass.IsNull())
	{
		return;
	}

	// Context별 핸들 생성
	FAbilityGrantedHandles& Handles = ContextHandles.FindOrAdd(ChangeContext);

	// GFCM에 Extension Handler 등록
	Handles.ExtensionRequestHandle = GFCM->AddExtensionHandler(
		TargetClass,
		UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
			this,
			&ThisClass::HandleActorExtension,
			ChangeContext
		)
	);
}

void UGameFeatureAction_AddAbilities::HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	// GameActorReady 이벤트에서만 처리
	if (EventName != UGameFrameworkComponentManager::NAME_GameActorReady)
	{
		return;
	}

	UCommonAbilitySystemComponent* ASC = GetAbilitySystemComponent(Actor);
	if (!ASC)
	{
		UE_LOG(GameFeatureAction_AddAbilitiesLog, Warning, TEXT("Actor [%s]에 AbilitySystemComponent가 없습니다"), *GetNameSafe(Actor));
		return;
	}

	// Context별 핸들 가져오기
	FAbilityGrantedHandles* Handles = ContextHandles.Find(ChangeContext);
	if (!Handles)
	{
		UE_LOG(GameFeatureAction_AddAbilitiesLog, Warning, TEXT("Context 핸들을 찾을 수 없습니다"));
		return;
	}

	// 이미 부여된 경우 스킵
	if (Handles->AbilitySpecHandles.Contains(Actor))
	{
		return;
	}

	TArray<FGameplayAbilitySpecHandle>& ActorHandles = Handles->AbilitySpecHandles.Add(Actor);

	for (const FCommonAbilityEntry& Entry : Abilities)
	{
		if (!Entry.Ability)
		{
			continue;
		}

		FGameplayAbilitySpec Spec(Entry.Ability, 1, INDEX_NONE, Actor);

		// InputTag가 있으면 DynamicSpecSourceTags에 추가
		if (Entry.InputTag.IsValid())
		{
			Spec.GetDynamicSpecSourceTags().AddTag(Entry.InputTag);
		}

		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
		if (!Handle.IsValid())
		{
			UE_LOG(GameFeatureAction_AddAbilitiesLog, Warning, TEXT("Ability [%s] 부여에 실패했습니다"), *GetNameSafe(Entry.Ability));
		}
		else
		{
			ActorHandles.Add(Handle);
		}
	}
}

UCommonAbilitySystemComponent* UGameFeatureAction_AddAbilities::GetAbilitySystemComponent(AActor* Actor) const
{
	if (!Actor)
	{
		return nullptr;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	return Cast<UCommonAbilitySystemComponent>(ASC);
}