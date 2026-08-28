// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "A1Projectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UGameplayEffect;
class UAbilitySystemComponent;

DECLARE_LOG_CATEGORY_EXTERN(A1ProjectileLog, Log, All);

/**
 * AA1Projectile
 *
 * 원거리 무기(지팡이 등)가 발사하는 투사체. 서버에서만 스폰되고(bReplicates + SetReplicateMovement로
 * 클라에 위치를 복제), 서버 권위에서만 충돌 판정과 데미지 GameplayEffect 적용을 수행한다.
 *
 * 발사한 어빌리티(UA1Ability_RangedWeaponAttack)가 SpawnActorDeferred 직후 Init()으로
 * 소스 ASC/데미지 GE/데미지 값을 주입하고 FinishSpawning한다. 무기별 데미지 값은
 * URangedWeaponInstance가 들고 있고, 이 액터 자체는 재사용 가능한 순수 "날아가서 맞으면 터지는" 로직만 담당한다.
 */
UCLASS(Abstract, Blueprintable)
class A1_API AA1Projectile : public AActor
{
	GENERATED_BODY()

public:
	AA1Projectile(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * 발사한 어빌리티가 SpawnActorDeferred 직후, FinishSpawning 이전에 호출한다(서버 전용).
	 * 데미지 적용에 필요한 정보를 주입한다. InScale은 액터 전체(콜리전 포함)에 적용되어 크기가
	 * 커질수록 히트박스도 함께 커진다. 메시 자체는 무기마다 다른 AA1Projectile 파생 BP를 만들어
	 * 디자인 타임에 지정한다(런타임 주입은 리플리케이션 비용 때문에 쓰지 않는다).
	 */
	void Init(UAbilitySystemComponent* InSourceASC, TSubclassOf<UGameplayEffect> InDamageEffectClass, float InDamage, float InSpeed, float InScale = 1.f);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 서버 전용. 유효한 대상이면 데미지 GE를 적용하고 GameplayCue를 실행한 뒤 스스로 파괴된다. */
	void ProcessHit(AActor* OtherActor, const FHitResult& HitResult);

	/**
	 * Init에서 서버가 설정한 액터 스케일(차징 크기 배수)은 SetActorScale3D만으로는 클라에 복제되지
	 * 않는다(Scale은 FRepMovement에 포함되지 않음). 그래서 Replicated 프로퍼티로 값을 들고, 이
	 * OnRep에서 클라이언트에 실제로 적용한다. 서버 자신은 OnRep이 호출되지 않으므로 Init에서
	 * 즉시 한 번 더 적용해준다.
	 */
	UFUNCTION()
	void OnRep_ActorScale();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A1|Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A1|Projectile")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A1|Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	/** 피격 시 발사자 ASC에서 실행할 GameplayCue. (기존 근접무기 임팩트 큐를 재사용해도 되고, 마법 전용 큐를 새로 만들어도 된다) */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Projectile", meta = (Categories = "GameplayCue"))
	FGameplayTag HitGameplayCueTag;

	/** 스폰 후 이 시간(초) 안에 아무것도 맞히지 못하면 스스로 파괴된다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Projectile")
	float MaxLifeSpan = 5.f;

private:
	/** Init에서 주입되는, 발사한 캐릭터의 ASC (데미지 적용의 Source). */
	TWeakObjectPtr<UAbilitySystemComponent> SourceASC;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	float Damage = 0.f;

	/** 중복 피격 방지 겸 파괴 순서 보장. */
	bool bHasHit = false;

	/** 액터 전체 스케일(차징에 의한 크기 배수). 스폰 시점에만 정해지고 이후 바뀌지 않으므로 InitialOnly로 복제한다. */
	UPROPERTY(ReplicatedUsing = OnRep_ActorScale)
	float ReplicatedActorScale = 1.f;
};
