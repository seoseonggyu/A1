// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "Game/CommonCharacter.h"
#include "Input/InputTypes.h"
#include "Interaction/A1Interactable.h"
#include "A1Character.generated.h"

class UInputMappingContext;

DECLARE_LOG_CATEGORY_EXTERN(A1CharacterLog, Log, All);


/**
 *
 * �� �ý����� �����ϴ� ĳ���� Ŭ�����Դϴ�.
 * �� ������ ���� PlayerController���� �����ɴϴ�.
 *
 * IA1Interactable을 구현해 다른 플레이어의 상호작용 대상이 될 수 있다.
 * CanInteract는 ASC의 Status.Death 루즈 태그로 사망(시체) 상태만 허용하므로, 시체 상태에서만
 * Interact가 가능하다. 사망 처리 자체는 UA1VitalSet::PostAttributeChange가 보내는
 * GameplayEvent.Death로 트리거되는 UA1Ability_Death가 전담한다(Status.Death 태그 설정 포함).
 * 실제 결과 처리(장비/인벤토리 열람 등)는 GatherInteractionOptions가 제공하는
 * InteractEventTag(GameplayEvent.Interact.Player)로 UA1Ability_Interact_Player에 위임한다.
 */
UCLASS()
class A1_API AA1Character : public ACommonCharacter, public IA1Interactable
{
	GENERATED_BODY()

public:
	AA1Character(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	//-----------------------------------------------------------------------------
	// IA1Interactable
	//-----------------------------------------------------------------------------

	virtual void GetHighlightComponents(TArray<UPrimitiveComponent*>& OutComponents) const override;
	virtual void GatherInteractionOptions(const FA1InteractionQuery& Query, TArray<FA1InteractionOption>& OutOptions) const override;
	virtual bool CanInteract(const FA1InteractionQuery& Query) const override;

private:
	//-----------------------------------------------------------------------------
	// 사망 시 장착 아이템 숨김 (Status.Death 태그 구독)
	//-----------------------------------------------------------------------------

	/**
	 * ASC의 Status.Death 태그를 구독해, 죽는 순간(또는 이미 죽은 상태로 리플리케이션되어 들어온 경우)
	 * 장착 중인 무기/장비의 스폰된 Actor를 숨긴다. 장착 해제가 아니라 은닉이므로 EquipmentComponent의
	 * 슬롯 데이터는 그대로 남아 시체 루팅 UI에서는 계속 "장착됨"으로 보인다.
	 *
	 * PlayerState의 ASC 리플리케이션이 아직 끝나지 않았을 수 있어(특히 원격 클라) 준비될 때까지 재시도한다.
	 */
	void TryTrackDeathStatusLocal();

	/** ASC를 찾아 Status.Death 태그 구독을 건다. 아직 ASC가 준비되지 않았으면 false. */
	bool SetupDeathStatusTrackingLocal();

	/** Status.Death 태그 카운트가 변할 때(및 구독 직후 현재 상태 반영 시) 호출된다. */
	void HandleDeathStatusChangedLocal(const FGameplayTag Tag, int32 NewCount);

	/**
	 * 사망 시 카메라를 자유 시점(UCommonCameraMode_FreeFly)으로 전환하고, 기존 이동/어빌리티용
	 * MappingContext를 모두 해제한 뒤 DeathFreeFlyMappingContext로 교체한다. 로컬 컨트롤 캐릭터에서만 호출.
	 */
	void ActivateDeathFreeFlyCameraLocal();

	FTimerHandle DeathStatusTrackingRetryTimerHandle;

	//-----------------------------------------------------------------------------
	// 사망 후 자유 시점 카메라용 입력 설정
	//-----------------------------------------------------------------------------

	/** 사망 후 활성화할 MappingContext (이동/어빌리티 MappingContext를 대체한다) */
	UPROPERTY(EditDefaultsOnly, Category = "Camera|FreeFly", meta = (AssetBundles = "Client"))
	TSoftObjectPtr<UInputMappingContext> DeathFreeFlyMappingContext;

	/** 사망 후 사용할 Native Input Action (시점 회전, 이동만 필요하며 Ability는 사용하지 않는다) */
	UPROPERTY(EditDefaultsOnly, Category = "Camera|FreeFly", meta = (TitleProperty = "{InputTag}"))
	TArray<FInputActionAndTag> DeathFreeFlyNativeInputActions;
};
