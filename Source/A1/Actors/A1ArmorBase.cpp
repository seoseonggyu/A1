#include "A1ArmorBase.h"

#include "Awaiters/Asset.h"
#include "Awaiters/Combinator.h"
#include "Awaiters/Delegate.h"
#include "Awaiters/Time.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1ArmorBase)

AA1ArmorBase::AA1ArmorBase(const FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	ArmorMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("ArmorMeshComponent");
	ArmorMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(ArmorMeshComponent);
}

void AA1ArmorBase::InitializeActor(TSoftObjectPtr<USkeletalMesh> InDefaultArmorMesh)
{
	LoadArmorMeshCoroutine(InDefaultArmorMesh);

}

TCoroTask<void> AA1ArmorBase::LoadArmorMeshCoroutine(TSoftObjectPtr<USkeletalMesh> ArmorMesh)
{
	USkeletalMesh* LoadedArmorMesh = co_await Coro::Async::LoadObject(this, ArmorMesh);
	if (LoadedArmorMesh)
	{
		ArmorMeshComponent->SetSkeletalMesh(LoadedArmorMesh);
	}
}
