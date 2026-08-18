// Copyright Epic Games, Inc. All Rights Reserved.

#include "Extension/Execute/ExtensionExecute_InitAbilitySystem.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/CommonAbilitySystemComponent.h"
#include "AbilitySystem/CommonAbilityTagRelationshipMapping.h"
#include "Awaiters/Asset.h"
#include "Game/CommonPlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtensionExecute_InitAbilitySystem)

void FExtensionExecute_InitAbilitySystem::OnActivate(AActor* Owner) const
{
	// Pawn 전용 Execute
	APawn* Pawn = Cast<APawn>(Owner);
	if (!Pawn)
	{
		return;
	}

	// PlayerState의 ASC 초기화
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn->GetPlayerState());
	if (!ASI)
	{
		return;
	}

	UCommonAbilitySystemComponent* ASC = Cast<UCommonAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
	if (!ASC)
	{
		return;
	}

	ASC->InitAbilityActorInfo(Pawn->GetPlayerState(), Pawn);
	ASC->PostNetInit();

	// PlayerController의 OnPostProcessInput에 ProcessAbilityInput 바인딩
	if (ACommonPlayerController* PC = Cast<ACommonPlayerController>(Pawn->GetController()))
	{
		ProcessInputDelegateHandle = PC->OnPostProcessInput.AddUObject(ASC, &UCommonAbilitySystemComponent::ProcessAbilityInput);
	}
	if (UCommonAbilityTagRelationshipMapping* Mapping = TagRelationshipMapping.Get())
	{
		if (IsValid(ASC) && Mapping)
		{
			ASC->SetTagRelationshipMapping(Mapping);
		}
	}
}


void FExtensionExecute_InitAbilitySystem::OnDeactivate(AActor* Owner) const
{
	APawn* Pawn = Cast<APawn>(Owner);
	if (!Pawn)
	{
		return;
	}

	// 델리게이트 바인딩 해제
	if (ProcessInputDelegateHandle.IsValid())
	{
		if (ACommonPlayerController* PC = Cast<ACommonPlayerController>(Pawn->GetController()))
		{
			PC->OnPostProcessInput.Remove(ProcessInputDelegateHandle);
		}
		ProcessInputDelegateHandle.Reset();
	}

	// ASC 정리
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn->GetPlayerState()))
	{
		if (UCommonAbilitySystemComponent* ASC = Cast<UCommonAbilitySystemComponent>(ASI->GetAbilitySystemComponent()))
		{
			ASC->ClearAbilityInput();
			ASC->ClearActorInfo();
		}
	}
}
