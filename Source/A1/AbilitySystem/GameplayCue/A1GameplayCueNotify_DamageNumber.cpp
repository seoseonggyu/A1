#include "AbilitySystem/GameplayCue/A1GameplayCueNotify_DamageNumber.h"

#include "Actors/A1DamageNumberActor.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayCueNotify_DamageNumber)

DEFINE_LOG_CATEGORY(A1GameplayCueNotify_DamageNumberLog);

bool UA1GameplayCueNotify_DamageNumber::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget)
	{
		return true;
	}

	UWorld* World = MyTarget->GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		// 표시할 로컬 뷰포트가 없는 서버에서는 스폰하지 않는다.
		return true;
	}

	if (!DamageNumberActorClass)
	{
		UE_LOG(A1GameplayCueNotify_DamageNumberLog, Warning, TEXT("[A1GameplayCueNotify_DamageNumber] DamageNumberActorClass가 설정되지 않았습니다"));
		return true;
	}

	const FVector2D RandomOffset2D = FMath::RandPointInCircle(RandomOffsetRadius);
	const FVector SpawnLocation = MyTarget->GetActorLocation() + FVector(RandomOffset2D.X, RandomOffset2D.Y, HeightOffset);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AA1DamageNumberActor* DamageNumberActor = World->SpawnActor<AA1DamageNumberActor>(DamageNumberActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (DamageNumberActor)
	{
		DamageNumberActor->InitializeDamageNumber(Parameters.RawMagnitude);
	}

	return true;
}
