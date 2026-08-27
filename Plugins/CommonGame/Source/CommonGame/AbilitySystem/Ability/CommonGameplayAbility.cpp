// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "AbilitySystem/CommonAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Camera/CommonCameraComponent.h"
#include "Camera/CommonCameraMode.h"
#include "Game/CommonCharacter.h"
#include "Game/CommonPlayerController.h"

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

ACommonPlayerController* UCommonGameplayAbility::GetCommonPlayerControllerFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<ACommonPlayerController>(CurrentActorInfo->PlayerController.Get()) : nullptr);
}

AController* UCommonGameplayAbility::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		if (AController* PC = CurrentActorInfo->PlayerController.Get())
		{
			return PC;
		}

		AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
		while (TestActor)
		{
			if (AController* C = Cast<AController>(TestActor))
			{
				return C;
			}

			if (APawn* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

ACommonCharacter* UCommonGameplayAbility::GetCommonCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<ACommonCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
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

	// TODO: TEMP TEST
	// OnSpawn 정책: 부여 즉시 활성화 시도
	// if (ActivationPolicy == ECommonAbilityActivationPolicy::OnSpawn)
	// {
	// 	if (UCommonAbilitySystemComponent* ASC = Cast<UCommonAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
	// 	{
	// 		ASC->TryActivateAbility(Spec.Handle);
	// 	}
	// }
	
	const bool bIsPredicting = (GetCurrentActivationInfo().ActivationMode == EGameplayAbilityActivationMode::Predicting);
	
	// Try to activate if activation policy is on spawn.
	if (ActorInfo && !Spec.IsActive() && !bIsPredicting && (ActivationPolicy == ECommonAbilityActivationPolicy::OnSpawn))
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

		// If avatar actor is torn off or about to die, don't try to activate until we get the new one.
		if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
		{
			const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
			const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

			const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
			const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

			if (bClientShouldActivate || bServerShouldActivate)
			{
				ASC->TryActivateAbility(Spec.Handle);
			}
		}
	}
}

void UCommonGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UCommonGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 하위 Ability가 ClearCameraMode를 깜빡 잊어도 항상 정리되도록 베이스에서 보장한다.
	ClearCameraMode();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCommonGameplayAbility::SetCameraMode(TSubclassOf<UCommonCameraMode> CameraModeClass)
{
	if (ACommonCharacter* Character = GetCommonCharacterFromActorInfo())
	{
		if (UCommonCameraComponent* CameraComponent = Character->GetCommonCameraComponent())
		{
			CameraComponent->SetAbilityCameraMode(CameraModeClass, CurrentSpecHandle);
			ActiveCameraModeClass = CameraModeClass;
		}
	}
}

void UCommonGameplayAbility::ClearCameraMode()
{
	if (!ActiveCameraModeClass)
	{
		return;
	}

	if (ACommonCharacter* Character = GetCommonCharacterFromActorInfo())
	{
		if (UCommonCameraComponent* CameraComponent = Character->GetCommonCameraComponent())
		{
			CameraComponent->ClearAbilityCameraMode(CurrentSpecHandle);
		}
	}

	ActiveCameraModeClass = nullptr;
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