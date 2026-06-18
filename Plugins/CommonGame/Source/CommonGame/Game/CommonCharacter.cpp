#include "CommonCharacter.h"
#include "Camera/CameraComponent.h"
#include "Camera/CommonCameraComponent.h"

ACommonCharacter::ACommonCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) // TODO: 이동 속도 관련
{
	// TODO: 메시 관련 생성해야함

	// TODO: 카메라 컴포넌트는 따로 빼서?
	CameraComponent = CreateDefaultSubobject<UCommonCameraComponent>(TEXT("CameraComponent"));

	// TODO: 다른 플레이어에게 보이게?

	// TODO: 이동 속도 관련
	
}

void ACommonCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void ACommonCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ACommonCharacter::SetAnimationData(TSubclassOf<UAnimInstance> AnimLayerClass) const
{
	// 애니메이션은 클라이언트에서만 필요합니다
	if (HasAuthority() || !AnimLayerClass)
	{
		return;
	}

	if (GetMesh() && AnimLayerClass)
	{
		GetMesh()->LinkAnimClassLayers(AnimLayerClass);
	}
}

void ACommonCharacter::ResetAnimationToDefault() const
{
	// TODO: 
}
