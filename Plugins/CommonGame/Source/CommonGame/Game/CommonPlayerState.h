// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ModularPlayerState.h"
#include "CommonPlayerState.generated.h"

// class UCommonAbilitySystemComponent; // TODO: 추가해야함

/**
 * CommonGame의 기본 PlayerState
 *
 * AModularPlayerState를 상속하여 모듈식 컴포넌트 추가를 지원합니다.
 * IAbilitySystemInterface를 구현하여 GAS 연동을 제공합니다.
 */
UCLASS()
class COMMONGAME_API ACommonPlayerState : public AModularPlayerState// , public IAbilitySystemInterface // TODO: 추가해야함
{
	GENERATED_BODY()

public:
	ACommonPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	// TODO: 추가해야함
	//virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** CommonAbilitySystemComponent를 반환합니다 */
	// TODO: 추가해야함
	//UCommonAbilitySystemComponent* GetCommonAbilitySystemComponent() const { return AbilitySystemComponent; }

private:
	/** Ability System Component */
	// TODO: 추가해야함
	//UPROPERTY(VisibleAnywhere, Category = "Abilities")
	//TObjectPtr<UCommonAbilitySystemComponent> AbilitySystemComponent;
};