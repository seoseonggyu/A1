// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CommonAttributeSet.generated.h"

/**
 * Attribute 접근자 매크로
 *
 * FGameplayAttributeData 프로퍼티에 대해 Getter, Setter, Initter 함수를 자동 생성합니다.
 * - GetPropertyAttribute(): FGameplayAttribute 반환
 * - GetProperty(): 현재 값 반환
 * - SetProperty(): 값 설정 (복제 지원 시 서버에서만)
 * - InitProperty(): 초기값 설정 (복제 전)
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * AttributeSet 베이스 클래스
 *
 * GAS(Gameplay Ability System)의 AttributeSet 기능을 확장합니다.
 * Iris PushModel을 지원하여 Attribute 변경 시 자동으로 dirty 마킹합니다.
 * 파생 클래스에서 게임별 Attribute를 정의할 수 있습니다.
 */
UCLASS(Abstract)
class COMMONGAME_API UCommonAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UCommonAttributeSet();

	//-----------------------------------------------------------------------------
	// UAttributeSet 오버라이드
	//-----------------------------------------------------------------------------

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	//-----------------------------------------------------------------------------
	// 초기화
	//-----------------------------------------------------------------------------

	/** ASC와 AttributeSet의 네트워크 복제가 완료된 시점에 호출됩니다 (클라이언트에서만 호출) */
	virtual void PostNetInit();
};