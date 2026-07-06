// Copyright Epic Games, Inc. All Rights Reserved.
#include "A1Ability_ChangeQuickBarSlot.h"
#include "Equipment/QuickBarComponent.h"
#include "AbilitySystemComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_ChangeQuickBarSlot)

UA1Ability_ChangeQuickBarSlot::UA1Ability_ChangeQuickBarSlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationPolicy = ECommonAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = ECommonAbilityActivationGroup::Exclusive_Blocking;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

bool UA1Ability_ChangeQuickBarSlot::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// SlotTag가 설정되어 있는지 확인
	if (!SlotTag.IsValid())
	{
		return false;
	}

	// QuickBarComponent가 있는지 확인
	const UQuickBarComponent* QuickBarComponent = GetQuickBarComponent(ActorInfo);
	if (!QuickBarComponent)
	{
		return false;
	}

	// 슬롯 변경이 가능한지 확인
	return QuickBarComponent->CanSetActiveSlot(SlotTag, SlotIndex);
}

void UA1Ability_ChangeQuickBarSlot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (UQuickBarComponent* QuickBarComponent = GetQuickBarComponent(ActorInfo))
	{
		// CanActivateAbility에서 이미 검증 완료, 바로 서버 RPC 호출
		QuickBarComponent->SetActiveSlotServer(SlotTag, SlotIndex);
	}

	// TODO: 여기서 아이템 장착하는 애니메이션 실행?

	// 슬롯 변경은 즉시 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

UQuickBarComponent* UA1Ability_ChangeQuickBarSlot::GetQuickBarComponent(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo)
	{
		return nullptr;
	}

	// QuickBarComponent는 PlayerController에 부착됨
	AController* Controller = Cast<AController>(ActorInfo->OwnerActor.Get());
	if (!Controller)
	{
		// AvatarActor의 Controller에서 찾기
		if (APawn* Pawn = Cast<APawn>(ActorInfo->AvatarActor.Get()))
		{
			Controller = Pawn->GetController();
		}
	}

	return Controller ? Controller->FindComponentByClass<UQuickBarComponent>() : nullptr;
}
