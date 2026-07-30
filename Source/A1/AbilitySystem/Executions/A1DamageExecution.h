#pragma once

#include "GameplayEffectExecutionCalculation.h"
#include "A1DamageExecution.generated.h"

/**
 * 데미지를 계산하는 GameplayEffectExecutionCalculation.
 *
 * BaseDamage는 어빌리티가 SetByCaller(A1GameplayTags::SetByCaller_BaseDamage)로 전달하고,
 * 여기서 최종 데미지를 계산해 UA1VitalSet의 Damage 메타 어트리뷰트에 더한다.
 * (Damage 메타는 UA1VitalSet::PostGameplayEffectExecute에서 Health로 분배된다)
 *
 * 방어력·치명타·팀 판정 등 감쇄 로직은 관련 어트리뷰트/시스템이 추가될 때 확장한다.
 */
UCLASS()
class A1_API UA1DamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UA1DamageExecution();

protected:
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
