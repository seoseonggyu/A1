// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_DropItem.h"

#include "A1GameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Interaction/Actors/A1Interactable_Pickup.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_DropItem)

DEFINE_LOG_CATEGORY(A1AbilityDropItemLog);

UA1Ability_DropItem::UA1Ability_DropItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 입력 키로 발동한다.
	ActivationPolicy = ECommonAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = ECommonAbilityActivationGroup::Independent;

	// 기본 드롭 클래스는 픽업. (메시 지정된 BP 픽업으로 GA 에셋에서 덮어써 사용)
	DropActorClass = AA1Interactable_Pickup::StaticClass();

	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_DropItem));
}

void UA1Ability_DropItem::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 실제 액터 스폰은 서버(Authority)에서만 수행한다. 클라는 예측 활성화만 하고 결과는 복제로 받는다.
	if (HasAuthority(&ActivationInfo))
	{
		SpawnDropActorAuth();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UA1Ability_DropItem::SpawnDropActorAuth()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	if (Avatar == nullptr || World == nullptr)
	{
		UE_LOG(A1AbilityDropItemLog, Warning, TEXT("SpawnDropActorAuth: Avatar 또는 World가 없어 스폰을 건너뜀."));
		return;
	}

	if (DropActorClass == nullptr)
	{
		UE_LOG(A1AbilityDropItemLog, Warning, TEXT("SpawnDropActorAuth: DropActorClass가 지정되지 않아 스폰을 건너뜀."));
		return;
	}

	const FTransform SpawnTransform = ComputeDropTransform(Avatar);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Avatar;
	SpawnParams.Instigator = Avatar->GetInstigator();
	// 겹치더라도 항상 스폰하되 가능하면 위치를 살짝 보정한다.
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AA1WorldInteractable* Dropped = World->SpawnActor<AA1WorldInteractable>(DropActorClass, SpawnTransform, SpawnParams);

	UE_LOG(A1AbilityDropItemLog, Log, TEXT("드롭 스폰: %s (by %s) at %s"), *GetNameSafe(Dropped), *GetNameSafe(Avatar), *SpawnTransform.GetLocation().ToCompactString());
}

FTransform UA1Ability_DropItem::ComputeDropTransform(const AActor* Avatar) const
{
	const FVector Forward = Avatar->GetActorForwardVector();
	FVector Location = Avatar->GetActorLocation() + Forward * DropForwardDistance;

	// 스폰 지점에서 아래로 라인 트레이스해 지면에 붙인다.
	if (const UWorld* World = GetWorld())
	{
		const FVector TraceStart = Location + FVector(0.f, 0.f, GroundTraceDistance * 0.5f);
		const FVector TraceEnd = Location - FVector(0.f, 0.f, GroundTraceDistance);

		FCollisionQueryParams Params(SCENE_QUERY_STAT(A1Ability_DropItem_Ground), false);
		Params.AddIgnoredActor(Avatar);

		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
		{
			Location = Hit.ImpactPoint;
		}
	}

	// 아바타의 Yaw만 반영해 눕지 않도록 회전을 구성한다.
	const FRotator Rotation(0.f, Avatar->GetActorRotation().Yaw, 0.f);
	return FTransform(Rotation, Location);
}
