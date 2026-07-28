// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/CommonAbilitySystemComponent.h"
#include "AbilitySystem/CommonAttributeSet.h"
#include "AbilitySystem/CommonAbilityTagRelationshipMapping.h"
#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "AbilitySystem/Ability/CommonAbilityTypes.h"
#include "CommonGameTags.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonAbilitySystemComponent)

UCommonAbilitySystemComponent::UCommonAbilitySystemComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Mixed 모드: 소유자에게 전체 정보, 시뮬레이트 프록시에게 최소 정보
	SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// 입력 초기화
	ClearAbilityInput();

	// 활성화 그룹 카운트 초기화
	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void UCommonAbilitySystemComponent::PostNetInit()
{
	if (bPostNetInitialized)
	{
		return;
	}

	bPostNetInitialized = true;

	// 모든 AttributeSet의 PostNetInit 호출
	for (UAttributeSet* Set : GetSpawnedAttributes())
	{
		if (UCommonAttributeSet* CommonSet = Cast<UCommonAttributeSet>(Set))
		{
			CommonSet->PostNetInit();
		}
	}
}

void UCommonAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	// 어빌리티 활성화가 성공한 직후
	Super::NotifyAbilityActivated(Handle, Ability);

	if (UCommonGameplayAbility* CommonAbility = Cast<UCommonGameplayAbility>(Ability))
	{
		AddAbilityToActivationGroup(CommonAbility->GetActivationGroup(), CommonAbility);
	}
}

void UCommonAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	//어빌리티 활성화가 끝난 직후
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (UCommonGameplayAbility* CommonAbility = Cast<UCommonGameplayAbility>(Ability))
	{
		RemoveAbilityFromActivationGroup(CommonAbility->GetActivationGroup(), CommonAbility);
	}
}

void UCommonAbilitySystemComponent::AbilityInputTagStarted(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability && (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
		{
			InputStartedSpecHandles.AddUnique(Spec.Handle);
		}
	}
}

void UCommonAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	// DynamicSpecSourceTags에 해당 태그가 있는 모든 Ability 검색
	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability && Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			InputPressedSpecHandles.AddUnique(Spec.Handle);
			InputHeldSpecHandles.AddUnique(Spec.Handle);
		}
	}
}

void UCommonAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability && Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			InputReleasedSpecHandles.AddUnique(Spec.Handle);
			InputHeldSpecHandles.Remove(Spec.Handle);
		}
	}
}

void UCommonAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	// 입력이 차단되었는지 확인
	if (HasMatchingGameplayTag(CommonGameTags::Gameplay_AbilityInputBlocked))
	{
		ClearAbilityInput();
		return;
	}

	// 활성화할 Ability 수집 (static으로 매 프레임 재할당 방지)
	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();

	// 1. WhileInputActive 정책: Hold 중인 Ability 활성화
	for (const FGameplayAbilitySpecHandle& Handle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle))
		{
			if (Spec->Ability && !Spec->IsActive())
			{
				if (const UCommonGameplayAbility* CommonAbility = Cast<UCommonGameplayAbility>(Spec->Ability))
				{
					if (CommonAbility->GetActivationPolicy() == ECommonAbilityActivationPolicy::WhileInputActive)
					{
						AbilitiesToActivate.AddUnique(Handle);
					}
				}
			}
		}
	}

	// 2. OnInputStarted 정책: Started된 Ability에 대한 활성화
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputStartedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				if (AbilitySpec->IsActive())
				{
					AbilitySecInputStarted(*AbilitySpec);
				}
			}
		}
	}

	// 3. OnInputTriggered 정책: Press된 Ability 활성화
	for (const FGameplayAbilitySpecHandle& Handle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle))
		{
			if (Spec->Ability)
			{
				Spec->InputPressed = true;

				if (Spec->IsActive())
				{
					// 이미 활성화된 경우: InputPressed 이벤트 전송
					AbilitySpecInputPressed(*Spec);
				}
				else
				{
					if (const UCommonGameplayAbility* CommonAbility = Cast<UCommonGameplayAbility>(Spec->Ability))
					{
						if (CommonAbility->GetActivationPolicy() == ECommonAbilityActivationPolicy::OnInputTriggered)
						{
							AbilitiesToActivate.AddUnique(Handle);
						}
					}
				}
			}
		}
	}

	// 4. 수집된 Ability 활성화
	for (const FGameplayAbilitySpecHandle& Handle : AbilitiesToActivate)
	{
		TryActivateAbility(Handle);
	}

	// 5. Release 처리: 활성화된 Ability에 입력 해제 알림
	for (const FGameplayAbilitySpecHandle& Handle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle))
		{
			if (Spec->Ability)
			{
				Spec->InputPressed = false;

				if (Spec->IsActive())
				{
					AbilitySpecInputReleased(*Spec);

					// WhileInputActive Ability는 입력 해제 시 종료
					if (UCommonGameplayAbility* CommonAbility = Cast<UCommonGameplayAbility>(Spec->Ability))
					{
						if (CommonAbility->GetActivationPolicy() == ECommonAbilityActivationPolicy::WhileInputActive)
						{
							CancelAbilityHandle(Handle);
						}
					}
				}
			}
		}
	}

	// 입력 상태 정리 (Started/Pressed/Released는 매 프레임 리셋)
	InputStartedSpecHandles.Reset();
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UCommonAbilitySystemComponent::ClearAbilityInput()
{
	InputStartedSpecHandles.Reset();
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

bool UCommonAbilitySystemComponent::IsActivationGroupBlocked(ECommonAbilityActivationGroup Group) const
{
	switch (Group)
	{
	case ECommonAbilityActivationGroup::Independent:
		// 독립 그룹은 항상 활성화 가능
		return false;

	case ECommonAbilityActivationGroup::Exclusive_Replaceable:
	case ECommonAbilityActivationGroup::Exclusive_Blocking:
		// Exclusive 그룹은 Blocking Ability가 활성화되어 있으면 차단
		return ActivationGroupCounts[static_cast<uint8>(ECommonAbilityActivationGroup::Exclusive_Blocking)] > 0;

	default:
		return false;
	}
}

void UCommonAbilitySystemComponent::AddAbilityToActivationGroup(ECommonAbilityActivationGroup Group, UCommonGameplayAbility* Ability)
{
	check(ActivationGroupCounts[static_cast<uint8>(Group)] < INT32_MAX);

	ActivationGroupCounts[static_cast<uint8>(Group)]++;

	switch (Group)
	{
	case ECommonAbilityActivationGroup::Independent:
		// 독립 그룹은 다른 Ability에 영향 없음
		break;

	case ECommonAbilityActivationGroup::Exclusive_Replaceable:
	case ECommonAbilityActivationGroup::Exclusive_Blocking:
		// Exclusive 그룹 진입 시 Replaceable Ability들을 취소
		// (Blocking이 활성화되어 있었다면 CanActivateAbility에서 이미 차단되어 여기까지 오지 못함)
		CancelActivationGroupAbilities(ECommonAbilityActivationGroup::Exclusive_Replaceable, Ability);
		break;

	default:
		break;
	}
}

void UCommonAbilitySystemComponent::RemoveAbilityFromActivationGroup(ECommonAbilityActivationGroup Group, UCommonGameplayAbility* Ability)
{
	check(ActivationGroupCounts[static_cast<uint8>(Group)] > 0);

	ActivationGroupCounts[static_cast<uint8>(Group)]--;
}

void UCommonAbilitySystemComponent::CancelActivationGroupAbilities(ECommonAbilityActivationGroup Group, UCommonGameplayAbility* IgnoreAbility)
{
	// 해당 그룹의 모든 활성 Ability 취소 (IgnoreAbility 제외)
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (!Spec.IsActive())
		{
			continue;
		}

		UCommonGameplayAbility* CommonAbility = Cast<UCommonGameplayAbility>(Spec.Ability);
		if (!CommonAbility || CommonAbility == IgnoreAbility)
		{
			continue;
		}

		if (CommonAbility->GetActivationGroup() == Group)
		{
			CancelAbilityHandle(Spec.Handle);
		}
	}
}

void UCommonAbilitySystemComponent::AbilitySecInputStarted(FGameplayAbilitySpec& Spec)
{
	if (Spec.IsActive())
	{
		if (UGameplayAbility* AbilityInstance = Spec.GetPrimaryInstance())
		{
			// 실행 중인 어빌리티 내부의 대기 중인 태스크에게 입력 신호를 전달한다.
			// RPC가 아니라 로컬 ASC 우편함에 기록 후 로컬 델리게이트를 브로드캐스트할 뿐이며,
			// 서버로의 전달은 신호를 받은 태스크가 OnStartCallback에서 수행한다.
			// (Handle + ActivationPredictionKey)가 태스크의 등록 키와 일치해야 신호가 닿는다.
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::GameCustom1, Spec.Handle, AbilityInstance->GetCurrentActivationInfo().GetActivationPredictionKey());
		}
	}
}


void UCommonAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	// UGameplayAbility::bReplicateInputDirectly는 지원하지 않는다.
	// WaitInputPress 계열 AbilityTask가 동작하도록 복제 이벤트로 대신 전달한다.
	if (Spec.IsActive())
	{
		if (UGameplayAbility* AbilityInstance = Spec.GetPrimaryInstance())
		{
			// InputPressed 이벤트 전송(여기서 서버로 복제하지 않음). 수신 태스크가 필요 시 서버로 복제한다.
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, AbilityInstance->GetCurrentActivationInfo().GetActivationPredictionKey());
		}
	}
}

void UCommonAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// UGameplayAbility::bReplicateInputDirectly는 지원하지 않는다.
	// WaitInputRelease 계열 AbilityTask가 동작하도록 복제 이벤트로 대신 전달한다.
	if (Spec.IsActive())
	{
		if (UGameplayAbility* AbilityInstance = Spec.GetPrimaryInstance())
		{
			// InputReleased 이벤트 전송(여기서 서버로 복제하지 않음). 수신 태스크가 필요 시 서버로 복제한다.
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, AbilityInstance->GetCurrentActivationInfo().GetActivationPredictionKey());
		}
	}
}

void UCommonAbilitySystemComponent::SetTagRelationshipMapping(UCommonAbilityTagRelationshipMapping* NewMapping)
{
	TagRelationshipMapping = NewMapping;
}

void UCommonAbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const
{
	if (TagRelationshipMapping)
	{
		TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
	}
}

void UCommonAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	FGameplayTagContainer ModifiedBlockTags = BlockTags;
	FGameplayTagContainer ModifiedCancelTags = CancelTags;

	if (TagRelationshipMapping)
	{
		// 매핑을 사용해 어빌리티 태그를 Block/Cancel 태그로 확장한다.
		TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, &ModifiedBlockTags, &ModifiedCancelTags);
	}

	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, ModifiedBlockTags, bExecuteCancelTags, ModifiedCancelTags);
}
