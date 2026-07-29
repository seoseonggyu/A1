// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "AbilitySystem/CommonAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
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

UCommonAbilitySystemComponent* UCommonGameplayAbility::GetCommonAbilitySystemComponentFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<UCommonAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr);
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

bool UCommonGameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 태그 관계 매핑을 통한 AbilityTags 확장을 처리하는 특화 버전

	bool bBlocked = false;
	bool bMissing = false;

	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
	const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

	// 이 어빌리티의 태그가 현재 차단 상태인지 확인 (UE5.8: AbilityTags → GetAssetTags())
	const FGameplayTagContainer& AssetTags = GetAssetTags();
	if (AbilitySystemComponent.AreAbilityTagsBlocked(AssetTags))
	{
		bBlocked = true;
	}

	const UCommonAbilitySystemComponent* CommonASC = Cast<UCommonAbilitySystemComponent>(&AbilitySystemComponent);
	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;

	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;

	// 매핑을 사용해 추가 요구/차단 태그를 확장한다.
	if (CommonASC)
	{
		CommonASC->GetAdditionalActivationTagRequirements(AssetTags, AllRequiredTags, AllBlockedTags);
	}

	// 확장된 요구/차단 태그를 검사한다.
	if (AllBlockedTags.Num() || AllRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;

		AbilitySystemComponentTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (AbilitySystemComponentTags.HasAny(AllBlockedTags))
		{
			bBlocked = true;
		}

		if (!AbilitySystemComponentTags.HasAll(AllRequiredTags))
		{
			bMissing = true;
		}
	}

	if (SourceTags != nullptr)
	{
		if (SourceBlockedTags.Num() || SourceRequiredTags.Num())
		{
			if (SourceTags->HasAny(SourceBlockedTags))
			{
				bBlocked = true;
			}

			if (!SourceTags->HasAll(SourceRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (TargetTags != nullptr)
	{
		if (TargetBlockedTags.Num() || TargetRequiredTags.Num())
		{
			if (TargetTags->HasAny(TargetBlockedTags))
			{
				bBlocked = true;
			}

			if (!TargetTags->HasAll(TargetRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (bBlocked)
	{
		if (OptionalRelevantTags && BlockedTag.IsValid())
		{
			OptionalRelevantTags->AddTag(BlockedTag);
		}
		return false;
	}
	if (bMissing)
	{
		if (OptionalRelevantTags && MissingTag.IsValid())
		{
			OptionalRelevantTags->AddTag(MissingTag);
		}
		return false;
	}

	return true;
}