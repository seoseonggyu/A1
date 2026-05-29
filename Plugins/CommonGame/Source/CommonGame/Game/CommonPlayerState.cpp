// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/CommonPlayerState.h"
// #include "AbilitySystem/CommonAbilitySystemComponent.h"  // TODO: 추가해야함
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonPlayerState)

ACommonPlayerState::ACommonPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// PlayerState의 기본 NetUpdateFrequency가 낮아서(1) ASC 데이터(Attribute, Tag, Effect) 동기화가 지연됩니다
	// ASC가 PlayerState에 있으므로 업데이트 빈도를 높여 즉각적인 동기화를 보장합니다
	SetNetUpdateFrequency(100.f);

	//AbilitySystemComponent = CreateDefaultSubobject<UCommonAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	//AbilitySystemComponent->SetIsReplicated(true);
}


// TODO: 추가해야함
//UAbilitySystemComponent* ACommonPlayerState::GetAbilitySystemComponent() const
//{
//	return AbilitySystemComponent;
//}