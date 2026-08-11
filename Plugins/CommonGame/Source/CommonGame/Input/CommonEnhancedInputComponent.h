// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "Input/InputTypes.h"
#include "CommonEnhancedInputComponent.generated.h"

COMMONGAME_API DECLARE_LOG_CATEGORY_EXTERN(CommonEnhancedInputComponentLog, Log, All);

//-----------------------------------------------------------------------------
// FAbilityInputBindingHandles
//-----------------------------------------------------------------------------

/** BindAbilityAction의 Start/Press/Release 바인딩 핸들 */
struct FAbilityInputBindingHandles
{
public:
	bool IsValid() const { return StartHandle != 0 && PressHandle != 0 && ReleaseHandle != 0; }

public:
	/** Start(Started) 바인딩 핸들 */
	uint32 StartHandle = 0;

	/** Press(Triggered) 바인딩 핸들 */
	uint32 PressHandle = 0;

	/** Release(Completed) 바인딩 핸들 */
	uint32 ReleaseHandle = 0;
};

//-----------------------------------------------------------------------------
// UCommonEnhancedInputComponent
//-----------------------------------------------------------------------------

/**
 * FInputActionAndTag 기반 입력 바인딩을 지원하는 Enhanced Input Component
 *
 * GameplayTag와 InputAction을 매핑한 FInputActionAndTag 구조체를 사용하여
 * 데이터 기반 입력 시스템 구현을 단순화합니다
 */
UCLASS()
class COMMONGAME_API UCommonEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	/** Native 입력 액션 매핑 설정 (이동, 시점 등) */
	void SetNativeInputActionMappings(const TArray<FInputActionAndTag>& InMappings);

	/** Ability 입력 액션 매핑 설정 (스킬 발동 등) */
	void SetAbilityInputActionMappings(const TArray<FInputActionAndTag>& InMappings);

	//-----------------------------------------------------------------------------
	// BindNativeAction - 태그로 액션 찾아서 바인딩
	//-----------------------------------------------------------------------------

	/**
	 * 파라미터 없는 콜백을 바인딩합니다
	 *
	 * 입력 발생 여부만 중요한 단순 트리거 입력에 사용됩니다
	 * 예: 점프, 재장전, 상호작용 등 on/off 방식의 입력
	 * @return 바인딩 핸들 (0이면 실패, 제거 시 사용)
	 */
	template <class UserClass, typename... VarTypes>
	uint32 BindNativeAction(
		const FGameplayTag& InputTag,
		ETriggerEvent TriggerEvent,
		UserClass* Object,
		TMemFunPtrType<false, UserClass, void(VarTypes...)>::Type Func,
		VarTypes... Vars)
	{
		if (const UInputAction* Action = FindNativeActionByTag(InputTag))
		{
			return BindAction(Action, TriggerEvent, Object, Func, Vars...).GetHandle();
		}

		UE_LOG(CommonEnhancedInputComponentLog, Warning, TEXT("BindNativeAction: [%s] 태그에 해당하는 InputAction을 찾을 수 없습니다"), *InputTag.ToString());
		return 0;
	}

	/**
	 * FInputActionValue 콜백을 바인딩합니다
	 *
	 * 입력 값(축 데이터)이 필요한 경우 사용됩니다
	 * 예: 이동(WASD -> FVector2D), 시점 회전(마우스 -> FVector2D)
	 * @return 바인딩 핸들 (0이면 실패, 제거 시 사용)
	 */
	template <class UserClass, typename... VarTypes>
	uint32 BindNativeAction(
		const FGameplayTag& InputTag,
		ETriggerEvent TriggerEvent,
		UserClass* Object,
		TMemFunPtrType<false, UserClass, void(const FInputActionValue&, VarTypes...)>::Type Func,
		VarTypes... Vars)
	{
		if (const UInputAction* Action = FindNativeActionByTag(InputTag))
		{
			return BindAction(Action, TriggerEvent, Object, Func, Vars...).GetHandle();
		}

		UE_LOG(CommonEnhancedInputComponentLog, Warning, TEXT("BindNativeAction: [%s] 태그에 해당하는 InputAction을 찾을 수 없습니다"), *InputTag.ToString());
		return 0;
	}

	/**
	 * FInputActionInstance 콜백을 바인딩합니다
	 *
	 * 입력 값과 함께 시간/상태 메타데이터가 필요한 경우 사용됩니다
	 * 예: 차징 공격(누른 시간에 따라 위력 변화), 홀드 입력 처리
	 * @return 바인딩 핸들 (0이면 실패, 제거 시 사용)
	 */
	template <class UserClass, typename... VarTypes>
	uint32 BindNativeAction(
		const FGameplayTag& InputTag,
		ETriggerEvent TriggerEvent,
		UserClass* Object,
		TMemFunPtrType<false, UserClass, void(const FInputActionInstance&, VarTypes...)>::Type Func,
		VarTypes... Vars)
	{
		if (const UInputAction* Action = FindNativeActionByTag(InputTag))
		{
			return BindAction(Action, TriggerEvent, Object, Func, Vars...).GetHandle();
		}

		UE_LOG(CommonEnhancedInputComponentLog, Warning, TEXT("BindNativeAction: [%s] 태그에 해당하는 InputAction을 찾을 수 없습니다"), *InputTag.ToString());
		return 0;
	}

	//-----------------------------------------------------------------------------
	// BindNativeActionByLambda - 람다 함수 바인딩
	//-----------------------------------------------------------------------------

	/**
	 * FInputActionValue 람다 콜백을 바인딩합니다
	 *
	 * UObject 없이 람다 함수를 직접 바인딩할 수 있습니다
	 * 주로 struct에서 입력 처리 시 사용됩니다
	 * @return 바인딩 핸들 (0이면 실패, 제거 시 사용)
	 */
	template <typename FuncType>
	uint32 BindNativeActionValueLambda(const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, FuncType&& Func)
	{
		if (const UInputAction* Action = FindNativeActionByTag(InputTag))
		{
			return BindActionValueLambda(Action, TriggerEvent, Forward<FuncType>(Func)).GetHandle();
		}

		UE_LOG(CommonEnhancedInputComponentLog, Warning, TEXT("BindNativeActionValueLambda: [%s] 태그에 해당하는 InputAction을 찾을 수 없습니다"), *InputTag.ToString());
		return 0;
	}

	/**
	 * FInputActionInstance 람다 콜백을 바인딩합니다
	 *
	 * UObject 없이 람다 함수를 직접 바인딩할 수 있습니다
	 * 입력 값과 함께 시간/상태 메타데이터가 필요한 경우 사용됩니다
	 * @return 바인딩 핸들 (0이면 실패, 제거 시 사용)
	 */
	template <typename FuncType>
	uint32 BindNativeActionInstanceLambda(const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, FuncType&& Func)
	{
		if (const UInputAction* Action = FindNativeActionByTag(InputTag))
		{
			return BindActionInstanceLambda(Action, TriggerEvent, Forward<FuncType>(Func)).GetHandle();
		}

		UE_LOG(CommonEnhancedInputComponentLog, Warning, TEXT("BindNativeActionInstanceLambda: [%s] 태그에 해당하는 InputAction을 찾을 수 없습니다"), *InputTag.ToString());
		return 0;
	}

	//-----------------------------------------------------------------------------
	// BindAbilityAction - Press/Release 자동 등록 + 태그 전달
	//-----------------------------------------------------------------------------

	/**
	 * 어빌리티용 입력을 바인딩합니다
	 *
	 * Start (Started), Press(Triggered), Completed(Release)를 자동으로 등록합니다
	 * 콜백에 어떤 입력인지 태그와 Press/Release 여부를 전달합니다
	 * @return Start/Press/Release 바인딩 핸들 (제거 시 사용)
	 */
	template <class UserClass>
	FAbilityInputBindingHandles BindAbilityAction(
		const FGameplayTag& InputTag,
		UserClass* Object,
		TMemFunPtrType<false, UserClass, void(FGameplayTag, bool)>::Type Func)
	{
		FAbilityInputBindingHandles Handles;

		if (const UInputAction* Action = FindAbilityActionByTag(InputTag))
		{
			// Start (Started)
			Handles.StartHandle = BindAction(Action, ETriggerEvent::Started, Object, Func, InputTag, true).GetHandle();

			// Press (Triggered)
			Handles.PressHandle = BindAction(Action, ETriggerEvent::Triggered, Object, Func, InputTag, true).GetHandle();

			// Release (Completed)
			Handles.ReleaseHandle = BindAction(Action, ETriggerEvent::Completed, Object, Func, InputTag, false).GetHandle();
		}
		else
		{
			UE_LOG(CommonEnhancedInputComponentLog, Warning, TEXT("BindAbilityAction: [%s] 태그에 해당하는 InputAction을 찾을 수 없습니다"), *InputTag.ToString());
		}

		return Handles;
	}

	/**
	 * 어빌리티용 입력을 람다로 바인딩합니다
	 *
	 * Start (Started), Press(Triggered), Completed(Release)를 자동으로 등록합니다
	 * UObject 없이 람다 함수를 직접 바인딩할 수 있습니다
	 * @param StartedFunc Start(Started) 콜백
	 * @param PressFunc Press(Triggered) 콜백
	 * @param ReleaseFunc Release(Completed) 콜백
	 * @return Start/Press/Release 바인딩 핸들 (제거 시 사용)
	 */
	template <typename StartedFuncType, typename PressFuncType, typename ReleaseFuncType>
	FAbilityInputBindingHandles BindAbilityActionLambda(const FGameplayTag& InputTag, StartedFuncType&& StartedFunc, PressFuncType&& PressFunc, ReleaseFuncType&& ReleaseFunc)
	{
		FAbilityInputBindingHandles Handles;

		if (const UInputAction* Action = FindAbilityActionByTag(InputTag))
		{
			// Start (Started)
			Handles.StartHandle =  BindActionInstanceLambda(Action, ETriggerEvent::Started, Forward<StartedFuncType>(StartedFunc)).GetHandle();

			// Press (Triggered)
			Handles.PressHandle = BindActionInstanceLambda(Action, ETriggerEvent::Triggered, Forward<PressFuncType>(PressFunc)).GetHandle();

			// Release (Completed)
			Handles.ReleaseHandle = BindActionInstanceLambda(Action, ETriggerEvent::Completed, Forward<ReleaseFuncType>(ReleaseFunc)).GetHandle();
		}
		else
		{
			UE_LOG(CommonEnhancedInputComponentLog, Warning, TEXT("BindAbilityActionLambda: [%s] 태그에 해당하는 InputAction을 찾을 수 없습니다"), *InputTag.ToString());
		}

		return Handles;
	}

private:
	/** Native 태그로 InputAction 찾기 */
	const UInputAction* FindNativeActionByTag(const FGameplayTag& InputTag) const;

	/** Ability 태그로 InputAction 찾기 */
	const UInputAction* FindAbilityActionByTag(const FGameplayTag& InputTag) const;

private:
	/** Native 입력 액션 매핑 (이동, 시점 등) */
	TArray<FInputActionAndTag> NativeInputActionMappings;

	/** Ability 입력 액션 매핑 (스킬 발동 등) */
	TArray<FInputActionAndTag> AbilityInputActionMappings;
};
