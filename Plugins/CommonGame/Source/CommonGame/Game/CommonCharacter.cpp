#include "CommonCharacter.h"

ACommonCharacter::ACommonCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) // TODO: 이동 속도 관련
{
	// TODO: 메시 관련 생성해야함

	// TODO: Top-Down 카메라 생성해야함

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
