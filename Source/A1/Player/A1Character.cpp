// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/A1Character.h"

#include "A1GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Actors/A1ArmorBase.h"
#include "Camera/CommonCameraComponent.h"
#include "Camera/CommonCameraMode_FreeFly.h"
#include "CommonGameTags.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Cosmetic/A1CosmeticManagerComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Equipment/EquipmentComponent.h"
#include "GameFramework/PlayerController.h"
#include "Input/CommonEnhancedInputComponent.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Physics/A1CollisionChannels.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Character)

DEFINE_LOG_CATEGORY(A1CharacterLog);

AA1Character::AA1Character(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FRotator initRotator = FRotator(0.0f, -90.0f, 0.0f);

	CameraComponent->SetRelativeRotation(initRotator);

	UArrowComponent* arrComp = GetArrowComponent();
	arrComp->SetRelativeRotation(initRotator);

	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -89.0f));
	MeshComp->SetRelativeRotation(initRotator);
	MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// 상호작용 커서 트레이스 채널에만 추가로 반응한다. 기존 Pawn 콜리전(캡슐)에는 영향 없음.
	GetCapsuleComponent()->SetCollisionResponseToChannel(A1_TraceChannel_Interaction, ECR_Block);

	// 투사체 채널의 기본 응답은 Block이지만(벽 등은 막아야 함), 캐릭터는 Overlap으로 맞아야
	// AA1Projectile::OnCollisionBeginOverlap이 발동해 피격 판정이 이루어진다.
	GetCapsuleComponent()->SetCollisionResponseToChannel(A1_TraceChannel_Projectile, ECR_Overlap);
}

void AA1Character::BeginPlay()
{
	Super::BeginPlay();

	TryTrackDeathStatusLocal();
}

void AA1Character::GetHighlightComponents(TArray<UPrimitiveComponent*>& OutComponents) const
{
	const UA1CosmeticManagerComponent* CosmeticCmp = UA1CosmeticManagerComponent::FindCosmeticManagerComponent(this);
	if (CosmeticCmp == nullptr)
	{
		return;
	}

	const TArray<TObjectPtr<UChildActorComponent>>& Slots = CosmeticCmp->GetCosmeticSlots();

	for (const TObjectPtr<UChildActorComponent>& ChildActorComponent : Slots)
	{
		if (ChildActorComponent == nullptr)
		{
			continue;
		}

		const AA1ArmorBase* ArmorActor = Cast<AA1ArmorBase>(ChildActorComponent->GetChildActor());
		if (ArmorActor == nullptr)
		{
			continue;
		}

		if (USkeletalMeshComponent* ArmorMesh = ArmorActor->GetMesh())
		{
			OutComponents.Add(ArmorMesh);
		}
	}
}

void AA1Character::GatherInteractionOptions(const FA1InteractionQuery& Query, TArray<FA1InteractionOption>& OutOptions) const
{
	FA1InteractionOption Option;
	Option.Interactable = TScriptInterface<IA1Interactable>(const_cast<AA1Character*>(this));
	Option.Title = FText::FromString(GetActorNameOrLabel());
	// CanInteract가 사망 상태만 통과시키므로, 여기 도달하는 대상은 항상 시체다. (Corpse 스텐실 값 사용)
	Option.HighlightStencil = 4;
	Option.InteractEventTag = A1GameplayTags::GameplayEvent_Interact_Player;
	OutOptions.Add(Option);
}

bool AA1Character::CanInteract(const FA1InteractionQuery& Query) const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC == nullptr)
	{
		return false;
	}

	return ASC->HasMatchingGameplayTag(A1GameplayTags::Status_Death);
}

void AA1Character::TryTrackDeathStatusLocal()
{
	if (SetupDeathStatusTrackingLocal())
	{
		return;
	}

	// PlayerState의 ASC 리플리케이션이 아직 끝나지 않았을 수 있다(특히 원격 클라). 준비될 때까지 재시도한다.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DeathStatusTrackingRetryTimerHandle, this, &ThisClass::TryTrackDeathStatusLocal, 0.2f, false);
	}
}

bool AA1Character::SetupDeathStatusTrackingLocal()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC == nullptr)
	{
		return false;
	}

	ASC->RegisterGameplayTagEvent(A1GameplayTags::Status_Death, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::HandleDeathStatusChangedLocal);

	// 이미 사망 상태로 리플리케이션되어 들어온 경우(늦게 관전/입장한 클라 등)를 대비해 현재 상태를 즉시 반영한다.
	HandleDeathStatusChangedLocal(A1GameplayTags::Status_Death, ASC->GetTagCount(A1GameplayTags::Status_Death));

	return true;
}

void AA1Character::HandleDeathStatusChangedLocal(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount <= 0)
	{
		return;
	}

	// 장착 데이터(EquipmentSlots/bActive)는 그대로 두고, 이미 스폰된 무기 Actor의 렌더링만 숨긴다.
	// 해제가 아니라 은닉이므로 시체 루팅 UI(EquipmentComponent 슬롯 조회)에는 계속 "장착됨"으로 보인다.
	if (UEquipmentComponent* EquipmentComponent = UEquipmentComponent::FindEquipmentComponent(this))
	{
		EquipmentComponent->SetAllEquipmentActorsHiddenLocal(true);
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
		Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		Capsule->SetCollisionResponseToChannel(A1_TraceChannel_Interaction, ECR_Block);
	}

	// 내가 조종하던 캐릭터가 죽었을 때만 카메라를 자유 시점으로 전환한다(시체를 보는 다른 클라는 대상 아님).
	if (IsLocallyControlled())
	{
		ActivateDeathFreeFlyCameraLocal();
	}
}

void AA1Character::ActivateDeathFreeFlyCameraLocal()
{
	UCommonCameraComponent* DeathCameraComponent = GetCommonCameraComponent();
	if (DeathCameraComponent == nullptr)
	{
		return;
	}

	// TopDown 대신 자유 시점 카메라 모드를 쓰도록 기본 카메라 모드를 교체한다.
	DeathCameraComponent->SetBaseCameraMode(UCommonCameraMode_FreeFly::StaticClass());

	APlayerController* PC = GetController<APlayerController>();
	UCommonEnhancedInputComponent* CommonInputComponent = Cast<UCommonEnhancedInputComponent>(InputComponent);
	if (PC == nullptr || !PC->GetLocalPlayer() || CommonInputComponent == nullptr)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (Subsystem == nullptr)
	{
		return;
	}

	// 탑다운 커서 조작용으로 띄워두던 마우스 커서를 숨기고 뷰포트에 캡처해서, 클릭/키 입력 없이도
	// 마우스를 움직이는 즉시 시점이 돌아가게 한다.
	PC->bShowMouseCursor = false;
	PC->SetInputMode(FInputModeGameOnly());

	// 이동/어빌리티용으로 쓰던 MappingContext를 전부 해제하고, 자유 시점 전용 MappingContext로 교체한다.
	// 사망 상태에서는 Ability를 쓰지 않으므로 Ability 입력은 다시 바인딩하지 않는다.
	Subsystem->ClearAllMappings();

	if (UInputMappingContext* LoadedContext = DeathFreeFlyMappingContext.Get())
	{
		Subsystem->AddMappingContext(LoadedContext, 0);
	}

	CommonInputComponent->SetNativeInputActionMappings(DeathFreeFlyNativeInputActions);

	CommonInputComponent->BindNativeActionValueLambda(
		CommonGameTags::Input_Native_Look,
		ETriggerEvent::Triggered,
		[DeathCameraComponent](const FInputActionValue& Value)
		{
			if (UCommonCameraMode_FreeFly* FreeFlyMode = Cast<UCommonCameraMode_FreeFly>(DeathCameraComponent->GetTopCameraMode()))
			{
				FreeFlyMode->AddLookInput(Value.Get<FVector2D>());
			}
		});

	CommonInputComponent->BindNativeActionValueLambda(
		CommonGameTags::Input_Native_Move,
		ETriggerEvent::Triggered,
		[DeathCameraComponent](const FInputActionValue& Value)
		{
			if (UCommonCameraMode_FreeFly* FreeFlyMode = Cast<UCommonCameraMode_FreeFly>(DeathCameraComponent->GetTopCameraMode()))
			{
				FreeFlyMode->AddMoveInput(Value.Get<FVector2D>());
			}
		});
}
