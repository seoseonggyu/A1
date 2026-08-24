// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_Interact_Player.h"

#include "A1GameplayTags.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/A1Interactable.h"
#include "Layout/CommonPrimaryGameLayout.h"
#include "UI/Loot/A1LootWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Interact_Player)

DEFINE_LOG_CATEGORY(A1AbilityInteractPlayerLog);

UA1Ability_Interact_Player::UA1Ability_Interact_Player(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// UA1Ability_Interact가 서버에서만 보내는 GameplayEvent로 트리거된다.
	ActivationPolicy = ECommonAbilityActivationPolicy::Manual;

	// 서버가 로컬로 발동을 결정하고, 그 활성화가 소유 클라에 복제되어 각자 자기 쪽 로직을 수행한다.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_Interact_Player));
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Interacting);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = A1GameplayTags::GameplayEvent_Interact_Player;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UA1Ability_Interact_Player::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* Interactor = GetAvatarActorFromActorInfo();
	AActor* TargetActor = (TriggerEventData != nullptr) ? const_cast<AActor*>(TriggerEventData->Target.Get()) : nullptr;

	if (Cast<IA1Interactable>(TargetActor) == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 위젯 표시는 순수 로컬 연출이므로 소유 클라에서만 수행한다. (서버 인스턴스는 결과 처리를 담당하지만
	// 지금은 열람 전용이라 서버가 할 일이 없다 — 실제 아이템 이동 기능이 추가되면 여기 서버 분기에 붙인다)
	if (HasAuthority(&ActivationInfo) == false)
	{
		ShowLootWidgetLocal(TargetActor);
	}

	UE_LOG(A1AbilityInteractPlayerLog, Log, TEXT("Player Interact 실행: Interactor=%s Target=%s"), *GetNameSafe(Interactor), *GetNameSafe(TargetActor));

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UA1Ability_Interact_Player::ShowLootWidgetLocal(AActor* TargetActor)
{
	if (LootWidgetClass == nullptr || TargetActor == nullptr)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetControllerFromActorInfo());
	UCommonPrimaryGameLayout* Layout = PC ? UCommonPrimaryGameLayout::GetPrimaryGameLayout(PC->GetLocalPlayer()) : nullptr;
	if (Layout == nullptr)
	{
		UE_LOG(A1AbilityInteractPlayerLog, Warning, TEXT("ShowLootWidgetLocal: PrimaryGameLayout을 찾을 수 없습니다"));
		return;
	}

	APawn* TargetPawn = Cast<APawn>(TargetActor);

	Layout->PushWidgetToLayerStack<UA1LootWidget>(WidgetLayerTag, LootWidgetClass, [TargetPawn](UA1LootWidget& Widget)
	{
		Widget.InitializeLoot(TargetPawn);
	});
}
