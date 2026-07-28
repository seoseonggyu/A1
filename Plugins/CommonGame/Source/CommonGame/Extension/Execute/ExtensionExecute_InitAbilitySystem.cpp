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
	
	// 태그 관계 매핑을 비동기 로딩하여 ASC에 주입 (코루틴 Owner = ASC)
	//LoadTagRelationshipMappingCoroutine(ASC);
	
}

TCoroTask<void> FExtensionExecute_InitAbilitySystem::LoadTagRelationshipMappingCoroutine(UCommonAbilitySystemComponent* ASC) const
{
	if (!TagRelationshipMapping)
	{
		co_return;
	}
	
	// SoftPtr는 awaiter로 값 복사되어 들어가므로, 재개 후 struct 상태에 접근하지 않는다.
	UCommonAbilityTagRelationshipMapping* Mapping = co_await Coro::Async::LoadObject(ASC, TagRelationshipMapping);

	// Owner(ASC)가 파괴되면 코루틴이 취소되므로, 재개 시 ASC는 유효하다.
	if (IsValid(ASC) && Mapping)
	{
		ASC->SetTagRelationshipMapping(Mapping);
	}

	co_return;
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
