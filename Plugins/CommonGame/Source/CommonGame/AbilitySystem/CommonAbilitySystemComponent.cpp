// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/CommonAbilitySystemComponent.h"
#include "AbilitySystem/CommonAttributeSet.h"
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
	Super::NotifyAbilityActivated(Handle, Ability);

	if (UCommonGameplayAbility* CommonAbility = Cast<UCommonGameplayAbility>(Ability))
	{
		AddAbilityToActivationGroup(CommonAbility->GetActivationGroup(), CommonAbility);
	}
}

void UCommonAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (UCommonGameplayAbility* CommonAbility = Cast<UCommonGameplayAbility>(Ability))
	{
		RemoveAbilityFromActivationGroup(CommonAbility->GetActivationGroup(), CommonAbility);
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

	// 2. OnInputTriggered 정책: Press된 Ability 활성화
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

	// 3. 수집된 Ability 활성화
	for (const FGameplayAbilitySpecHandle& Handle : AbilitiesToActivate)
	{
		TryActivateAbility(Handle);
	}

	// 4. Release 처리: 활성화된 Ability에 입력 해제 알림
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

	// 입력 상태 정리 (Pressed/Released는 매 프레임 리셋)
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UCommonAbilitySystemComponent::ClearAbilityInput()
{
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