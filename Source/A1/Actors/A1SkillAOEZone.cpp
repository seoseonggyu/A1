// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actors/A1SkillAOEZone.h"

#include "A1GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Game/CommonCharacter.h"
#include "GameplayEffect.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1SkillAOEZone)

DEFINE_LOG_CATEGORY(A1SkillAOEZoneLog);

AA1SkillAOEZone::AA1SkillAOEZone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	// 서버 전용 판정/타이머 액터. 표시는 전부 디버그 드로우이므로 클라에 존재할 필요가 없다.
	bReplicates = false;
}

void AA1SkillAOEZone::Init(UAbilitySystemComponent* InSourceASC, TSubclassOf<UGameplayEffect> InDamageEffectClass, TSubclassOf<UGameplayEffect> InSlowEffectClass,
	float InRadius, float InTickDamage, float InTickInterval, int32 InTickCount, float InSlowAmount, float InSlowDuration)
{
	SourceASC = InSourceASC;
	DamageEffectClass = InDamageEffectClass;
	SlowEffectClass = InSlowEffectClass;
	Radius = InRadius;
	TickDamage = InTickDamage;
	TickInterval = FMath::Max(InTickInterval, 0.1f);
	TickCount = FMath::Max(InTickCount, 1);
	SlowAmount = InSlowAmount;
	SlowDuration = InSlowDuration;
}

void AA1SkillAOEZone::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == false)
	{
		return;
	}

	// 첫 지연을 0으로 줘서 스폰 즉시 1틱이 나가게 한다. (기본 SetTimer는 InRate만큼 지난 뒤에야
	// 첫 콜백이 실행되므로, 그대로 두면 장판이 생기고 TickInterval초 동안 아무 피해도 없는 것처럼 보인다)
	GetWorldTimerManager().SetTimer(ZoneTimerHandle, this, &ThisClass::TickZone, TickInterval, true, 0.f);
}

void AA1SkillAOEZone::TickZone()
{
	++CurrentTick;

	const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn) };
	const TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> OverlappedActors;

	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), Radius, ObjectTypes, ACommonCharacter::StaticClass(), ActorsToIgnore, OverlappedActors);

	for (AActor* OverlappedActor : OverlappedActors)
	{
		ApplyDamageAndSlow(OverlappedActor);
	}

#if ENABLE_DRAW_DEBUG
	// 실제로 판정이 들어간 순간의 반경을 눈으로 확인한다. (서버 프로세스에서만 렌더링됨)
	DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 24, DebugColor, false, TickInterval * 0.9f);
#endif

	if (CurrentTick >= TickCount)
	{
		GetWorldTimerManager().ClearTimer(ZoneTimerHandle);
		Destroy();
	}
}

void AA1SkillAOEZone::ApplyDamageAndSlow(AActor* TargetActor) const
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	UAbilitySystemComponent* SourceASCPtr = SourceASC.Get();
	if (TargetASC == nullptr || SourceASCPtr == nullptr)
	{
		return;
	}

	// 시전자 자신은 자신의 장판에 피해를 입지 않는다.
	if (TargetASC == SourceASCPtr)
	{
		return;
	}

	if (DamageEffectClass)
	{
		FGameplayEffectContextHandle EffectContextHandle = SourceASCPtr->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = SourceASCPtr->MakeOutgoingSpec(DamageEffectClass, 1.f, EffectContextHandle);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(A1GameplayTags::SetByCaller_BaseDamage, TickDamage);
			TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	if (SlowEffectClass && SlowAmount > 0.f)
	{
		FGameplayEffectContextHandle SlowContextHandle = SourceASCPtr->MakeEffectContext();
		FGameplayEffectSpecHandle SlowSpecHandle = SourceASCPtr->MakeOutgoingSpec(SlowEffectClass, 1.f, SlowContextHandle);
		if (SlowSpecHandle.IsValid())
		{
			// GE 에셋 자체의 Duration 값과 무관하게 SlowDuration으로 강제한다(Sprint의 RecoveryBlockEffectClass와 같은 방식).
			SlowSpecHandle.Data->SetDuration(SlowDuration, true);
			SlowSpecHandle.Data->SetSetByCallerMagnitude(A1GameplayTags::SetByCaller_SlowAmount, -FMath::Abs(SlowAmount));
			TargetASC->ApplyGameplayEffectSpecToSelf(*SlowSpecHandle.Data.Get());
		}
	}
}
