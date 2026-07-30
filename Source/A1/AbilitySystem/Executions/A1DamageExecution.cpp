#include "A1DamageExecution.h"

#include "A1GameplayTags.h"
#include "AbilitySystem/A1VitalSet.h"
#include "GameplayEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1DamageExecution)

UA1DamageExecution::UA1DamageExecution()
{
	// A1은 아직 포착할 방어/능력치 어트리뷰트가 없어 캡처 목록을 비워 둔다.
	// 방어력·저항 등이 생기면 여기에 FGameplayEffectAttributeCaptureDefinition을 추가한다.
}

void UA1DamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// BaseDamage는 어빌리티에서 SetByCaller로 전달한다. (UA1Ability_MeleeWeaponAttack::ProcessHitResult)
	const float BaseDamage = Spec.GetSetByCallerMagnitude(A1GameplayTags::SetByCaller_BaseDamage, /*WarnIfNotFound=*/true, /*DefaultIfNotFound=*/0.f);

	// TODO: 방어력/치명타/팀 판정/약점(PhysicalMaterial) 등 감쇄 로직 추가
	const float DamageDone = FMath::Max(BaseDamage, 0.f);

	if (DamageDone > 0.f)
	{
		// Damage 메타 어트리뷰트에 더한다. Health로의 실제 반영은 UA1VitalSet::PostGameplayEffectExecute가 담당한다.
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UA1VitalSet::GetDamageAttribute(), EGameplayModOp::Additive, DamageDone));
	}
#endif // WITH_SERVER_CODE
}
