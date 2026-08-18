// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionUserWidget.h"
#include "GameplayTagContainer.h"
#include "A1SkillCooldownWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;
class UAbilitySystemComponent;
class UEquipmentComponent;
class UEquipmentInstance;

DECLARE_LOG_CATEGORY_EXTERN(A1SkillCooldownWidgetLog, Log, All);

/**
 * 스킬 슬롯 하나(Q, E 등)를 표시하는 위젯
 *
 * 손에 든 메인 장비가 SkillInputTag(예: Input.Ability.Attack.Skill.1)로 바인딩된 Ability를 가지고
 * 있을 때만 아이콘을 보여줍니다(예: 검엔 스킬이 있고 물약엔 없음). 쿨타임 중에는 아이콘을 흐리게
 * 하고 남은 시간을 표시하다가, 끝나면 다시 선명해집니다.
 *
 * 파페돌처럼 이 위젯을 여러 번 배치하고, 인스턴스마다 SkillInputTag/CooldownTag를 다르게 지정합니다.
 * MVVM을 쓰지 않고 EquipmentComponent/ASC를 직접 구독합니다 (Equipment/Inventory 위젯과 동일한 방식).
 */
UCLASS(Abstract)
class A1_API UA1SkillCooldownWidget : public UCommonExtensionUserWidget
{
	GENERATED_BODY()

protected:
	//-----------------------------------------------------------------------------
	// UUserWidget 오버라이드
	//-----------------------------------------------------------------------------

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	/**
	 * ASC/EquipmentComponent를 찾을 때까지 짧은 간격으로 재시도합니다.
	 *
	 * 이 위젯은 Inventory/Equipment 창과 달리 HUD에 항상 떠 있어서, Pawn 빙의나 ASC/Equipment
	 * 컴포넌트 리플리케이션(특히 클라이언트)이 끝나기 전에 NativeConstruct가 먼저 실행될 수 있다.
	 */
	void TryStartTracking();

	/** ASC/EquipmentComponent를 찾아 구독합니다. 실패하면(아직 준비 안 됨) false를 반환합니다 */
	bool SetupTracking();

	/** 구독을 해제합니다 (재시도 전 상태 초기화 용도로도 사용, RetryTimerHandle은 건드리지 않음) */
	void TearDown();

	/** 손에 든 메인 장비가 바뀔 때 호출되어 이 스킬의 보유 여부를 갱신합니다 */
	void HandleMainEquippedItemChanged(UEquipmentInstance* NewMainItem);

	/** CooldownTag의 부여(NewCount>0)·해제(NewCount==0)를 처리합니다 */
	void HandleCooldownTagChanged(const FGameplayTag Tag, int32 NewCount);

	/** 1초마다 호출되어 남은 쿨타임을 갱신합니다 */
	void TickCooldown();

	/** CooldownTag를 부여한 활성 GameplayEffect의 남은 시간을 초 단위로 반환합니다 (없으면 0) */
	float GetRemainingCooldownSeconds() const;

	/** 현재 상태(보유 여부/쿨타임)에 맞춰 아이콘·텍스트를 갱신합니다 */
	void RefreshVisual();

protected:
	//-----------------------------------------------------------------------------
	// 설정 (BP 인스턴스마다 지정)
	//-----------------------------------------------------------------------------

	/** 이 슬롯이 담당하는 스킬의 입력 태그. 손에 든 장비의 Ability 목록과 대조해 보유 여부를 판단합니다 */
	UPROPERTY(EditAnywhere, Category = "Skill", meta = (Categories = "Input.Ability"))
	FGameplayTag SkillInputTag;

	/** 이 슬롯의 쿨타임 태그 */
	UPROPERTY(EditAnywhere, Category = "Skill", meta = (Categories = "Cooldown"))
	FGameplayTag CooldownTag;

	/** 쿨타임 중 아이콘 불투명도 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CooldownOpacity = 0.4f;

	//-----------------------------------------------------------------------------
	// BindWidget (BP에서 배치)
	//-----------------------------------------------------------------------------

	/** 스킬 아이콘 (텍스처는 BP에서 직접 지정) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Icon;

	/** 남은 쿨타임 숫자 (쿨타임 아닐 땐 빈 텍스트) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Cooldown;

private:
	TWeakObjectPtr<UAbilitySystemComponent> ASC;
	TWeakObjectPtr<UEquipmentComponent> EquipmentComponent;

	FDelegateHandle MainEquippedChangedHandle;
	FTimerHandle CooldownTimerHandle;
	FTimerHandle RetryTimerHandle;

	/** 현재 보유 중인 스킬의 아이콘. 장착 아이템이 바뀌면 HandleMainEquippedItemChanged에서 갱신됩니다 */
	UPROPERTY()
	TObjectPtr<UTexture2D> CurrentIcon = nullptr;

	bool bHasSkill = false;
	bool bOnCooldown = false;
	int32 CooldownRemaining = 0;
};
