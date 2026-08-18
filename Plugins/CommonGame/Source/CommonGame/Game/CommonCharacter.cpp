#include "CommonCharacter.h"
#include "Camera/CameraComponent.h"
#include "Camera/CommonCameraComponent.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Equipment/EquipmentInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonCharacter)

DEFINE_LOG_CATEGORY(CommonCharacterLog);

ACommonCharacter::ACommonCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CameraComponent = CreateDefaultSubobject<UCommonCameraComponent>(TEXT("CameraComponent"));
}

void ACommonCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetAnimationData(DefaultAnimInstanceClass);

}

void ACommonCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

UAbilitySystemComponent* ACommonCharacter::GetAbilitySystemComponent() const
{
	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetPlayerState()))
	{
		return ASI->GetAbilitySystemComponent();
	}
	return nullptr;
}

void ACommonCharacter::SetAnimationData(TSubclassOf<UAnimInstance> AnimLayerClass, UEquipmentInstance* Requester) const
{
	// 애니메이션은 클라이언트에서만 필요합니다
	if (HasAuthority() || !AnimLayerClass)
	{
		return;
	}

	if (GetMesh())
	{
		GetMesh()->LinkAnimClassLayers(AnimLayerClass);
		CurrentAnimLayerOwner = Requester;
	}
}

void ACommonCharacter::ResetAnimationToDefault(UEquipmentInstance* Requester) const
{
	// Requester가 지정됐는데 현재 AnimLayer 소유자가 아니면, 이미 다른 장비가 새 AnimLayer를
	// 적용한 뒤라는 뜻이므로 리셋을 무시한다. (자세한 배경은 헤더의 ResetAnimationToDefault 주석 참고)
	if (Requester && CurrentAnimLayerOwner.Get() != Requester)
	{
		return;
	}

	SetAnimationData(DefaultAnimInstanceClass, nullptr);
}

