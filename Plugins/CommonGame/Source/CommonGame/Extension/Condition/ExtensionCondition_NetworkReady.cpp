// Copyright Epic Games, Inc. All Rights Reserved.

#include "Extension/Condition/ExtensionCondition_NetworkReady.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtensionCondition_NetworkReady)

bool FExtensionCondition_NetworkReady::IsSatisfied(AActor* Owner) const
{
	// Pawn 전용 Condition
	APawn* Pawn = Cast<APawn>(Owner);
	if (!Pawn)
	{
		return false;
	}

	// Controller 존재 확인
	AController* Controller = Pawn->GetController();
	if (!Controller)
	{
		return false;
	}

	// PlayerController 타입 확인 (옵션)
	if (bRequirePlayerController && !Cast<APlayerController>(Controller))
	{
		return false;
	}

	// PlayerState 존재 확인
	if (!Pawn->GetPlayerState())
	{
		return false;
	}

	// Controller-PlayerState 페어링 검증 (옵션)
	if (bRequirePlayerStateLinked)
	{
		if (!Controller->PlayerState || Controller->PlayerState->GetOwner() != Controller)
		{
			return false;
		}
	}

	// AbilitySystemComponent 복제 확인 (옵션)
	if (bRequireAbilitySystemReady)
	{
		const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn->GetPlayerState());
		if (!ASI)
		{
			return false;
		}

		UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
		if (!ASC)
		{
			return false;
		}

		// RequiredAttributeSets에 지정된 AttributeSet 복제 확인
		for (const TSubclassOf<UAttributeSet>& AttributeSetClass : RequiredAttributeSets)
		{
			if (AttributeSetClass && !ASC->GetAttributeSet(AttributeSetClass))
			{
				return false;
			}
		}
	}

	return true;
}
