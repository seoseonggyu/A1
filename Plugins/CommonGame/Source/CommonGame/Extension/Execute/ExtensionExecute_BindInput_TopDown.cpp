// Copyright Epic Games, Inc. All Rights Reserved.

#include "Extension/Execute/ExtensionExecute_BindInput_TopDown.h"
#include "AbilitySystemInterface.h"
//#include "AbilitySystem/CommonAbilitySystemComponent.h" // TODO: Ability
#include "CommonGameTags.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Input/CommonEnhancedInputComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtensionExecute_BindInput_TopDown)

void FExtensionExecute_BindInput_TopDown::OnActivate(AActor* Owner) const
{
	// Pawn 전용 Execute
	APawn* Pawn = Cast<APawn>(Owner);
	APlayerController* PC = Pawn ? Pawn->GetController<APlayerController>() : nullptr;
	if (!PC || !PC->GetLocalPlayer())
	{
		return;
	}

	WeakPawn = Pawn;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	UCommonEnhancedInputComponent* InputComponent = Cast<UCommonEnhancedInputComponent>(Pawn->InputComponent);

	if (!Subsystem || !InputComponent)
	{
		return;
	}

	// MappingContext 등록
	if (UInputMappingContext* LoadedContext = MappingContext.Get())
	{
		Subsystem->AddMappingContext(LoadedContext, Priority);
	}

	// Input 매핑 설정
	InputComponent->SetNativeInputActionMappings(NativeInputActions);
	InputComponent->SetAbilityInputActionMappings(AbilityInputActions);

	// Move 입력 바인딩 (람다 사용)
	if (uint32 MoveHandle = InputComponent->BindNativeActionValueLambda(
		CommonGameTags::Input_Native_Move,
		ETriggerEvent::Triggered,
		[this](const FInputActionValue& Value) { Input_Move(Value); }))
	{
		BindingHandles.Add(MoveHandle);
	}

	// Look 입력 바인딩 (람다 사용)
	if (uint32 LookHandle = InputComponent->BindNativeActionValueLambda(
		CommonGameTags::Input_Native_Look,
		ETriggerEvent::Triggered,
		[this](const FInputActionValue& Value) { Input_Look(Value); }))
	{
		BindingHandles.Add(LookHandle);
	}

	// Ability 입력 바인딩 - ASC로 라우팅
	for (const FInputActionAndTag& Mapping : AbilityInputActions)
	{
		if (!Mapping.InputTag.IsValid())
		{
			continue;
		}

		const FGameplayTag InputTag = Mapping.InputTag;

		FAbilityInputBindingHandles Handles = InputComponent->BindAbilityActionLambda(
			InputTag,
			[this, InputTag](const FInputActionInstance&) { Input_AbilityPressed(InputTag); },
			[this, InputTag](const FInputActionInstance&) { Input_AbilityReleased(InputTag); }
		);

		if (Handles.IsValid())
		{
			BindingHandles.Add(Handles.PressHandle);
			BindingHandles.Add(Handles.ReleaseHandle);
		}
	}
}

void FExtensionExecute_BindInput_TopDown::OnDeactivate(AActor* Owner) const
{
	APawn* Pawn = Cast<APawn>(Owner);
	APlayerController* PC = Pawn ? Pawn->GetController<APlayerController>() : nullptr;
	if (!PC || !PC->GetLocalPlayer())
	{
		return;
	}

	// 입력 바인딩 제거
	if (UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>(Pawn->InputComponent))
	{
		for (uint32 Handle : BindingHandles)
		{
			InputComponent->RemoveBindingByHandle(Handle);
		}
	}
	BindingHandles.Empty();

	// MappingContext 제거
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		if (UInputMappingContext* LoadedContext = MappingContext.Get())
		{
			Subsystem->RemoveMappingContext(LoadedContext);
		}
	}

	WeakPawn.Reset();
}

void FExtensionExecute_BindInput_TopDown::Input_Move(const FInputActionValue& InputActionValue) const
{
	APawn* Pawn = WeakPawn.Get();
	if (!Pawn)
	{
		return;
	}

	AController* Controller = Pawn->GetController();
	if (!Controller)
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

	if (Value.X != 0.0f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		Pawn->AddMovementInput(MovementDirection, Value.X);
	}

	if (Value.Y != 0.0f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
		Pawn->AddMovementInput(MovementDirection, Value.Y);
	}
}

void FExtensionExecute_BindInput_TopDown::Input_Look(const FInputActionValue& InputActionValue) const
{
	APawn* Pawn = WeakPawn.Get();
	if (!Pawn)
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

void FExtensionExecute_BindInput_TopDown::Input_AbilityPressed(FGameplayTag InputTag) const
{
	APawn* Pawn = WeakPawn.Get();
	if (!Pawn)
	{
		return;
	}

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn))
	{
		// TODO: Ability
		/*if (UCommonAbilitySystemComponent* ASC = Cast<UCommonAbilitySystemComponent>(ASI->GetAbilitySystemComponent()))
		{
			ASC->AbilityInputTagPressed(InputTag);
		}*/
	}
}

void FExtensionExecute_BindInput_TopDown::Input_AbilityReleased(FGameplayTag InputTag) const
{
	APawn* Pawn = WeakPawn.Get();
	if (!Pawn)
	{
		return;
	}

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn))
	{
		// TODO: Ability
		/*if (UCommonAbilitySystemComponent* ASC = Cast<UCommonAbilitySystemComponent>(ASI->GetAbilitySystemComponent()))
		{
			ASC->AbilityInputTagReleased(InputTag);
		}*/
	}
}
