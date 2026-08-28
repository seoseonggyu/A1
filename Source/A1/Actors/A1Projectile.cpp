// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actors/A1Projectile.h"

#include "A1GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayEffect.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Physics/A1CollisionChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Projectile)

DEFINE_LOG_CATEGORY(A1ProjectileLog);

AA1Projectile::AA1Projectile(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(A1_TraceChannel_Projectile);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->SetCanEverAffectNavigation(false);
	SetRootComponent(CollisionComponent);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetCanEverAffectNavigation(false);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->UpdatedComponent = CollisionComponent;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
}

void AA1Projectile::Init(UAbilitySystemComponent* InSourceASC, TSubclassOf<UGameplayEffect> InDamageEffectClass, float InDamage, float InSpeed, float InScale)
{
	SourceASC = InSourceASC;
	DamageEffectClass = InDamageEffectClass;
	Damage = InDamage;

	ProjectileMovementComponent->InitialSpeed = InSpeed;
	ProjectileMovementComponent->MaxSpeed = InSpeed;
	ProjectileMovementComponent->Velocity = GetActorForwardVector() * InSpeed;

	// 스케일(차징 크기 배수)은 Replicated 프로퍼티에 담아 클라에 복제한다. 서버 자신은 OnRep이
	// 호출되지 않으므로 OnRep_ActorScale을 직접 한 번 호출해 즉시 적용한다.
	ReplicatedActorScale = InScale;
	OnRep_ActorScale();
}

void AA1Projectile::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);
		CollisionComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnCollisionBeginOverlap);
		SetLifeSpan(MaxLifeSpan);
	}
}

void AA1Projectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 스폰 시 한 번만 정해지고 이후 바뀌지 않는 값이라 InitialOnly로 복제 비용을 아낀다.
	DOREPLIFETIME_CONDITION(AA1Projectile, ReplicatedActorScale, COND_InitialOnly);
}

void AA1Projectile::OnRep_ActorScale()
{
	SetActorScale3D(FVector(ReplicatedActorScale));
}

void AA1Projectile::OnCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasHit || OtherActor == nullptr || OtherActor == this || OtherActor == GetInstigator())
	{
		return;
	}

	ProcessHit(OtherActor, SweepResult);
}

// 서버 전용(HasAuthority일 때만 오버랩 바인딩됨). 데미지 GE를 대상 ASC에 직접 적용한다.
// UGameplayAbility가 아니라 순수 Actor라 MakeOutgoingGameplayEffectSpec 대신 ASC의 MakeOutgoingSpec을 직접 쓴다.
void AA1Projectile::ProcessHit(AActor* OtherActor, const FHitResult& HitResult)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor);
	UAbilitySystemComponent* SourceASCPtr = SourceASC.Get();

	// 발사자 자신(같은 ASC)은 무시한다. (예: 캐릭터 캡슐과 무기 소켓이 겹쳐 스폰 직후 자기 자신과 오버랩되는 경우)
	if (TargetASC != nullptr && TargetASC == SourceASCPtr)
	{
		return;
	}

	bHasHit = true;
	CollisionComponent->SetGenerateOverlapEvents(false);
	ProjectileMovementComponent->StopMovementImmediately();

	if (SourceASCPtr && HitGameplayCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = HitResult.ImpactPoint;
		CueParams.Normal = HitResult.ImpactNormal;
		CueParams.PhysicalMaterial = HitResult.PhysMaterial;
		SourceASCPtr->ExecuteGameplayCue(HitGameplayCueTag, CueParams);
	}

	if (SourceASCPtr && TargetASC && DamageEffectClass)
	{
		FGameplayEffectContextHandle EffectContextHandle = SourceASCPtr->MakeEffectContext();
		EffectContextHandle.AddHitResult(HitResult);

		FGameplayEffectSpecHandle SpecHandle = SourceASCPtr->MakeOutgoingSpec(DamageEffectClass, 1.f, EffectContextHandle);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(A1GameplayTags::SetByCaller_BaseDamage, Damage);
			TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	Destroy();
}
