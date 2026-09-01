// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "A1SkillAOEZone.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;

DECLARE_LOG_CATEGORY_EXTERN(A1SkillAOEZoneLog, Log, All);

/**
 * AA1SkillAOEZone
 *
 * UA1Ability_Skill_AOE가 확정된 커서 위치에 서버 전용으로 스폰하는 장판. 복제되지 않으며
 * (표시는 전부 디버그 드로우), TickInterval마다 반경 내 캐릭터에게 데미지 GE와 슬로우 GE를
 * 직접 적용한 뒤 TickCount회를 채우면 스스로 파괴된다. UA1Ability_Skill_AOE와 달리 어빌리티
 * 수명과 무관하게 독립적으로 지속되어, 시전자는 확정 즉시 다시 자유롭게 행동할 수 있다.
 */
UCLASS(Abstract, Blueprintable)
class A1_API AA1SkillAOEZone : public AActor
{
	GENERATED_BODY()

public:
	AA1SkillAOEZone(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 스폰 직후(서버 전용), FinishSpawning 이전에 호출해 장판 수치를 주입한다. */
	void Init(UAbilitySystemComponent* InSourceASC, TSubclassOf<UGameplayEffect> InDamageEffectClass, TSubclassOf<UGameplayEffect> InSlowEffectClass,
		float InRadius, float InTickDamage, float InTickInterval, int32 InTickCount, float InSlowAmount, float InSlowDuration);

protected:
	virtual void BeginPlay() override;

private:
	/** TickInterval마다 호출. 반경 내 캐릭터를 찾아 데미지/슬로우를 적용하고, TickCount에 도달하면 스스로 파괴된다. */
	void TickZone();

	/** 대상 ASC에 데미지 GE(SetByCaller.BaseDamage)와 슬로우 GE(SetByCaller.SlowAmount)를 직접 적용한다. 순수 Actor라 어빌리티 컨텍스트 없이 ASC의 MakeOutgoingSpec을 쓴다. */
	void ApplyDamageAndSlow(AActor* TargetActor) const;

private:
	TWeakObjectPtr<UAbilitySystemComponent> SourceASC;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> SlowEffectClass;

	float Radius = 300.f;
	float TickDamage = 10.f;
	float TickInterval = 1.f;
	int32 TickCount = 3;
	float SlowAmount = 0.5f;
	float SlowDuration = 1.5f;

	/** 지금까지 실행된 틱 횟수. TickCount에 도달하면 파괴된다. */
	int32 CurrentTick = 0;

	FTimerHandle ZoneTimerHandle;

	/** 데미지가 실제로 들어간 지점을 눈으로 확인하기 위한 디버그 색상. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|AOE Zone|Debug")
	FColor DebugColor = FColor::Red;
};
