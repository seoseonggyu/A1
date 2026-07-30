#include "A1AnimNotifyState_PerformTrace.h"

#include "A1GameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Equipment/EquipmentComponent.h"
#include "Player/A1Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1AnimNotifyState_PerformTrace)

DEFINE_LOG_CATEGORY(A1AnimNotifyState_PerformTraceLog);

UA1AnimNotifyState_PerformTrace::UA1AnimNotifyState_PerformTrace(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif
}

void UA1AnimNotifyState_PerformTrace::NotifyBegin(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComponent, Animation, TotalDuration, EventReference);

	// 이전에 남은 상태가 있으면 정리하고 새로 시작한다.
	ActiveContexts.Remove(MeshComponent);

	if (MeshComponent->GetOwnerRole() != ExecuteNetRole)
		return;

	AA1Character* Character = Cast<AA1Character>(MeshComponent->GetOwner());
	if (Character == nullptr)
		return;

	UEquipmentComponent* EquipmentComp = UEquipmentComponent::FindEquipmentComponent(Character);
	if (EquipmentComp == nullptr)
		return;

	// 장착된 무기 액터와 그 액터의 트레이스용 CollisionBox를 이 메시의 컨텍스트에 캐시한다.
	AActor* Weapon = EquipmentComp->GetEquipmentInstance(A1GameplayTags::Equipment_Slot_Weapon);
	if (Weapon == nullptr)
		return;

	UBoxComponent* Box = FindCollisionBox(Weapon);
	if (Box == nullptr)
	{
		UE_LOG(A1AnimNotifyState_PerformTraceLog, Warning, TEXT("무기 액터 %s 에서 CollisionBox를 찾지 못했습니다."), *Weapon->GetName());
		return;
	}

	FTraceContext& Context = ActiveContexts.Add(MeshComponent);
	Context.WeaponActor = Weapon;
	Context.CollisionBox = Box;
}

void UA1AnimNotifyState_PerformTrace::NotifyTick(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComponent, Animation, FrameDeltaTime, EventReference);

	if (MeshComponent->GetOwnerRole() != ExecuteNetRole)
		return;

	FTraceContext* Context = ActiveContexts.Find(MeshComponent);
	if (Context == nullptr || Context->CollisionBox.IsValid() == false)
		return;

	PerformTrace(MeshComponent, *Context);
}

void UA1AnimNotifyState_PerformTrace::NotifyEnd(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComponent, Animation, EventReference);

	if (MeshComponent->GetOwnerRole() == ExecuteNetRole)
	{
		if (FTraceContext* Context = ActiveContexts.Find(MeshComponent))
		{
			// 마지막 구간까지 한 번 더 검사한다.
			if (Context->CollisionBox.IsValid())
			{
				PerformTrace(MeshComponent, *Context);
			}
		}
	}

	// 이 메시의 트레이스 상태를 제거한다. (다른 캐릭터의 상태에는 영향 없음)
	ActiveContexts.Remove(MeshComponent);
}

void UA1AnimNotifyState_PerformTrace::PerformTrace(USkeletalMeshComponent* MeshComponent, FTraceContext& Context)
{
	UBoxComponent* Box = Context.CollisionBox.Get();
	if (Box == nullptr)
		return;

	UWorld* World = MeshComponent->GetWorld();
	if (World == nullptr)
		return;

	const FTransform BoxTransform = Box->GetComponentTransform();
	const FVector BoxExtent = Box->GetScaledBoxExtent();
	const FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxExtent);

	// 검출할 오브젝트 타입 (지정이 없으면 Pawn)
	FCollisionObjectQueryParams ObjectParams;
	if (TraceParams.DetectionObjectTypes.Num() > 0)
	{
		for (const TEnumAsByte<EObjectTypeQuery>& ObjectType : TraceParams.DetectionObjectTypes)
		{
			ObjectParams.AddObjectTypesToQuery(UEngineTypes::ConvertToCollisionChannel(ObjectType));
		}
	}
	else
	{
		ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(A1PerformTrace), false);
	QueryParams.AddIgnoredActor(MeshComponent->GetOwner());
	QueryParams.AddIgnoredActor(Context.WeaponActor.Get());

	// 블로킹 없이 박스 오버랩만 검사한다.
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(Overlaps, BoxTransform.GetLocation(), BoxTransform.GetRotation(), ObjectParams, BoxShape, QueryParams);

	// 이번에 새로 맞은 대상만 HitResult로 만든다. (같은 구간 중복 히트 방지)
	TArray<FHitResult> NewHitResults;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (HitActor == nullptr || Context.HitActors.Contains(HitActor))
			continue;

		Context.HitActors.Add(HitActor);

		FHitResult HitResult;
		HitResult.HitObjectHandle = FActorInstanceHandle(HitActor);
		HitResult.Component = Overlap.GetComponent();
		HitResult.Item = Overlap.ItemIndex;
		HitResult.Location = BoxTransform.GetLocation();
		HitResult.ImpactPoint = BoxTransform.GetLocation();
		NewHitResults.Add(HitResult);
	}

#if ENABLE_DRAW_DEBUG
	if (TraceDebugParams.bDrawDebugShape)
	{
		const FColor Color = (NewHitResults.Num() > 0) ? TraceDebugParams.HitColor : TraceDebugParams.TraceColor;
		DrawDebugBox(World, BoxTransform.GetLocation(), BoxExtent, BoxTransform.GetRotation(), Color, false, TraceDebugParams.DrawDuration);
	}
#endif

	if (NewHitResults.Num() == 0 || EventTag.IsValid() == false)
		return;

	// 새로 맞은 대상들을 TargetData로 묶어 소유 캐릭터에게 게임플레이 이벤트로 전달한다.
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	for (const FHitResult& HitResult : NewHitResults)
	{
		FGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FGameplayAbilityTargetData_SingleTargetHit();
		NewTargetData->HitResult = HitResult;
		TargetDataHandle.Add(NewTargetData);
	}

	FGameplayEventData EventData;
	EventData.TargetData = TargetDataHandle;
	EventData.Instigator = Context.WeaponActor.Get();

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComponent->GetOwner(), EventTag, EventData);
}

UBoxComponent* UA1AnimNotifyState_PerformTrace::FindCollisionBox(AActor* Actor) const
{
	if (Actor == nullptr)
		return nullptr;

	// 태그가 지정되면 해당 태그가 붙은 BoxComponent를 우선 사용한다.
	if (TraceParams.CollisionComponentTag != NAME_None)
	{
		TArray<UBoxComponent*> Boxes;
		Actor->GetComponents<UBoxComponent>(Boxes);
		for (UBoxComponent* Box : Boxes)
		{
			if (Box->ComponentHasTag(TraceParams.CollisionComponentTag))
				return Box;
		}
	}

	// 태그가 없거나 못 찾으면 첫 번째 BoxComponent를 사용한다.
	return Actor->FindComponentByClass<UBoxComponent>();
}
