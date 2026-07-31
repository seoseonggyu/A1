#include "A1Ability_Sprint_Check.h"

#include "A1GameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/A1VitalSet.h"
#include "Game/CommonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Sprint_Check)

UA1Ability_Sprint_Check::UA1Ability_Sprint_Check(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 서버 RPC(RequestSprintServer)를 사용하려면 어빌리티 인스턴스가 복제되어야 한다.
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;

	ActivationPolicy = ECommonAbilityActivationPolicy::OnInputTriggered;

	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_Sprint_Check));

	// 기본 판정 대상은 UA1VitalSet의 Stamina. 필요 시 BP/DataAsset에서 교체 가능.
	SprintCostAttribute = UA1VitalSet::GetStaminaAttribute();
}

void UA1Ability_Sprint_Check::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 입력을 소유한 쪽에서만 판정한다. (서버 원격 사본은 여기서 종료 신호를 기다린다)
	if (IsLocallyControlled() == false)
		return;

	ACommonCharacter* Character = GetCommonCharacterFromActorInfo();
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (Character == nullptr || Movement == nullptr || ASC == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	// 탑뷰: 캐릭터가 이동 방향으로 자동 회전하므로 '전방 이동'은 곧 '이동 입력 존재'와 같다.
	const bool bIsMoving = Character->GetLastMovementInputVector().IsNearlyZero() == false;

	bool bFoundAttribute = false;
	const float Stamina = UAbilitySystemBlueprintLibrary::GetFloatAttributeFromAbilitySystemComponent(ASC, SprintCostAttribute, bFoundAttribute);

	if (Movement->IsFalling() || bIsMoving == false || bFoundAttribute == false || Stamina <= MinStaminaToStart)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	// 로컬(예측/호스트)에서 먼저 Active를 발동한다.
	SendSprintActiveEventLocal();

	if (HasAuthority(&CurrentActivationInfo))
	{
		// 호스트/스탠드얼론: 로컬 발동이 곧 서버 발동이므로 바로 종료한다.
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	else
	{
		// 소유 클라: 서버에 권위 발동을 요청한다. (서버가 발동 후 Check를 종료한다)
		RequestSprintServer();
	}
}

void UA1Ability_Sprint_Check::RequestSprintServer_Implementation()
{
	SendSprintActiveEventLocal();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UA1Ability_Sprint_Check::SendSprintActiveEventLocal()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (AvatarActor == nullptr)
		return;

	FGameplayEventData Payload;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AvatarActor, A1GameplayTags::Ability_Sprint_Active, Payload);
}
