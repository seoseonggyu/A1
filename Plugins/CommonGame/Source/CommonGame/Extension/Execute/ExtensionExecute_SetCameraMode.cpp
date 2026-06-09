// Copyright Epic Games, Inc. All Rights Reserved.

#include "Extension/Execute/ExtensionExecute_SetCameraMode.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Game/CommonCharacter.h"
#include "Camera/CommonCameraMode.h"
#include "Camera/CommonCameraComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtensionExecute_SetCameraMode)

void FExtensionExecute_SetCameraMode::OnActivate(AActor* Owner) const
{
	// Pawn 전용 Execute
	APawn* Pawn = Cast<APawn>(Owner);
	APlayerController* PC = Pawn ? Pawn->GetController<APlayerController>() : nullptr;
	if (!PC)
	{
		return;
	}

	ACommonCharacter* Character = Cast<ACommonCharacter>(Pawn);
	if (UCommonCameraComponent* CameraComponent = Character ? Character->GetCommonCameraComponent() : nullptr)
	{
		CameraComponent->DetermineCameraModeDelegate.BindLambda(
			[this]() -> TSubclassOf<UCommonCameraMode>
			{
				return CameraModeClass;
			}
		);
	}
}

void FExtensionExecute_SetCameraMode::OnDeactivate(AActor* Owner) const
{
	APawn* Pawn = Cast<APawn>(Owner);
	if (!Pawn)
	{
		return;
	}

	// TODO: 카메라 모드 제거
	//if (bIsActive)
	//{
	//	if (ACommonCharacter* Character = Cast<ACommonCharacter>(Pawn))
	//	{
	//		 if (UGameplayCameraComponent* CameraComponent = Character->GetCameraComponent())
	//		 {
	//		 	CameraComponent->DeactivateCamera(true);
	//		 	CameraComponent->CameraReference.SetCameraAsset(nullptr);
	//		 }
	//	}

	//	bIsActive = false;
	//}
}
