// Copyright Epic Games, Inc. All Rights Reserved.

#include "Action/GameFeatureAction_AddAttributes.h"
#include "AbilitySystem/CommonAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AttributeSet.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFeaturesSubsystem.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddAttributes)

DEFINE_LOG_CATEGORY(GameFeatureAction_AddAttributesLog);

UGameFeatureAction_AddAttributes::UGameFeatureAction_AddAttributes()
{
	// AttributeSet 추가는 서버에서만 실행 (ASC가 SpawnedAttributes를 클라이언트에 복제함)
	bClientAction = false;
	bServerAction = true;
}

void UGameFeatureAction_AddAttributes::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	FGameFeatureStateChangeContext StateChangeContext(Context);

	// 해당 Context의 핸들만 제거
	if (FAttributeSetGrantedHandles* Handles = ContextHandles.Find(StateChangeContext))
	{
		// GFCM 핸들 해제
		Handles->ExtensionRequestHandle.Reset();

		// 부여된 AttributeSet 제거
		for (auto& Pair : Handles->AttributeSets)
		{
			if (AActor* Actor = Pair.Key.Get())
			{
				if (UCommonAbilitySystemComponent* ASC = GetAbilitySystemComponent(Actor))
				{
					for (const TWeakObjectPtr<UAttributeSet>& SetPtr : Pair.Value)
					{
						if (UAttributeSet* Set = SetPtr.Get())
						{
							ASC->RemoveSpawnedAttribute(Set);
						}
					}
				}
			}
		}

		ContextHandles.Remove(StateChangeContext);
	}
}

void UGameFeatureAction_AddAttributes::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
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
	FAttributeSetGrantedHandles& Handles = ContextHandles.FindOrAdd(ChangeContext);

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

void UGameFeatureAction_AddAttributes::HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	// GameActorReady 이벤트에서만 처리
	if (EventName != UGameFrameworkComponentManager::NAME_GameActorReady)
	{
		return;
	}

	UCommonAbilitySystemComponent* ASC = GetAbilitySystemComponent(Actor);
	if (!ASC)
	{
		UE_LOG(GameFeatureAction_AddAttributesLog, Warning, TEXT("Actor [%s]에 AbilitySystemComponent가 없습니다"), *GetNameSafe(Actor));
		return;
	}

	// Context별 핸들 가져오기
	FAttributeSetGrantedHandles* Handles = ContextHandles.Find(ChangeContext);
	if (!Handles)
	{
		UE_LOG(GameFeatureAction_AddAttributesLog, Warning, TEXT("Context 핸들을 찾을 수 없습니다"));
		return;
	}

	// 이미 부여된 경우 스킵
	if (Handles->AttributeSets.Contains(Actor))
	{
		return;
	}

	TArray<TWeakObjectPtr<UAttributeSet>>& ActorSets = Handles->AttributeSets.Add(Actor);

	for (const TSubclassOf<UAttributeSet>& SetClass : AttributeSetClasses)
	{
		if (!SetClass)
		{
			continue;
		}

		// 이미 해당 AttributeSet이 있는지 확인
		if (ASC->GetAttributeSet(SetClass))
		{
			UE_LOG(GameFeatureAction_AddAttributesLog, Log, TEXT("AttributeSet [%s]이 이미 존재합니다"), *GetNameSafe(SetClass));
			continue;
		}

		// AttributeSet 생성 및 ASC에 추가
		UAttributeSet* NewSet = NewObject<UAttributeSet>(ASC->GetOwner(), SetClass);
		if (!NewSet)
		{
			UE_LOG(GameFeatureAction_AddAttributesLog, Warning, TEXT("AttributeSet [%s] 생성에 실패했습니다"), *GetNameSafe(SetClass));
			continue;
		}

		ASC->AddSpawnedAttribute(NewSet);
		ActorSets.Add(NewSet);
	}
}

UCommonAbilitySystemComponent* UGameFeatureAction_AddAttributes::GetAbilitySystemComponent(const AActor* Actor) const
{
	if (!Actor)
	{
		return nullptr;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	return Cast<UCommonAbilitySystemComponent>(ASC);
}
