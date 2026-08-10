// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_Test.h"

#include "A1GameplayTags.h"
#include "Engine/Engine.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Test)

DEFINE_LOG_CATEGORY(A1Ability_TestLog);

UA1Ability_Test::UA1Ability_Test(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 스폰(부여) 직후 자동 활성화.
	ActivationPolicy = ECommonAbilityActivationPolicy::OnSpawn;
	ActivationGroup = ECommonAbilityActivationGroup::Independent;

	// 클라이언트에서만 실행되며 서버로 활성화 RPC를 보내지 않는다.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_Test));
}

bool UA1Ability_Test::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 서버 프로세스나 원격 폰에서는 어떤 경로로도 활성화되지 않도록 최종 방어선을 둔다.
	if (IsClientLocalContext(ActorInfo) == false)
	{
		return false;
	}

	// 스폰당 1회만 발동한다. (스펙 복제/아바타 세팅 양쪽 경로에서 시도되므로 중복을 막는다)
	if (IsInstantiated() && bActivatedForCurrentAvatar)
	{
		return false;
	}

	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UA1Ability_Test::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	// 새 캐릭터가 붙었으므로(최초 스폰/리스폰) 다시 1회 발동할 수 있게 한다.
	// 실제 활성화 시도는 UCommonAbilitySystemComponent::TryActivateLocalOnlyAbilitiesOnSpawn이 수행한다.
	if (IsInstantiated())
	{
		bActivatedForCurrentAvatar = false;
	}
}

void UA1Ability_Test::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bActivatedForCurrentAvatar = true;

	const AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;

	UE_LOG(A1Ability_TestLog, Log, TEXT("%s | Avatar=[%s] NetMode=%d"), *TestMessage, *GetNameSafe(AvatarActor), AvatarActor ? static_cast<int32>(AvatarActor->GetNetMode()) : -1);

	if (bPrintToScreen && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.0f, FColor::Cyan, FString::Printf(TEXT("%s | %s"), *TestMessage, *GetNameSafe(AvatarActor)));
	}

	// 단발성 테스트이므로 바로 종료한다. 서버로 나가는 통신이 없어야 하므로 종료도 복제하지 않는다.
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

bool UA1Ability_Test::IsClientLocalContext(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (ActorInfo == nullptr)
	{
		return false;
	}

	const AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (AvatarActor == nullptr)
	{
		return false;
	}

	// 데디케이티드/리슨 서버 프로세스에서는 실행하지 않는다. (스탠드얼론도 서버 권한이므로 제외)
	if (AvatarActor->GetNetMode() != NM_Client)
	{
		return false;
	}

	// 같은 클라이언트에 존재하는 다른 플레이어의 폰(시뮬레이티드 프록시)은 제외한다.
	return ActorInfo->IsLocallyControlled();
}
