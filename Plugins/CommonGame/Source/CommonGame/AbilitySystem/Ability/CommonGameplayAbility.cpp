// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "AbilitySystem/CommonAbilitySystemComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonGameplayAbility)

UCommonGameplayAbility::UCommonGameplayAbility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Ability 자체는 복제하지 않음
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;

	// 소유자당 1개 인스턴스
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 클라이언트 예측
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// 클라이언트/서버 모두 실행 가능
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

bool UCommonGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// ActivationGroup 체크
	const UCommonAbilitySystemComponent* CommonASC = Cast<UCommonAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (CommonASC && CommonASC->IsActivationGroupBlocked(ActivationGroup))
	{
		return false;
	}

	return true;
}

void UCommonGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// OnSpawn 정책: 부여 즉시 활성화 시도
	if (ActivationPolicy == ECommonAbilityActivationPolicy::OnSpawn)
	{
		if (UCommonAbilitySystemComponent* ASC = Cast<UCommonAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
		{
			ASC->TryActivateAbility(Spec.Handle);
		}
	}
}

void UCommonGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);
}