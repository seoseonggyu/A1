// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularCharacter.h"
#include "AbilitySystemInterface.h"
#include "CommonCharacter.generated.h"

class UCommonCameraComponent;
class USkeletalMeshComponent;

DECLARE_LOG_CATEGORY_EXTERN(CommonCharacterLog, Log, All);

/**
 * Experience 시스템과 ActorExtension을 지원하는 Character 베이스 클래스
 *
 * GameFrameworkComponentManager에 등록되어 모듈식 컴포넌트 추가를 지원합니다.
 * ActorExtension은 ActorExtensionWorldSubsystem에서 중앙 관리됩니다.
 */

UCLASS()
class COMMONGAME_API ACommonCharacter : public AModularCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ACommonCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	/** PlayerState의 AbilitySystemComponent를 반환합니다 */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 카메라 컴포넌트를 반환합니다 */
	UCommonCameraComponent* GetCommonCameraComponent() const { return CameraComponent; }

	void SetAnimationData(TSubclassOf<UAnimInstance> AnimLayerClass) const;
	virtual void ResetAnimationToDefault() const;


protected:
	/** Top-Down 카메라 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCommonCameraComponent> CameraComponent;
	
	/** 적용할 애니메이션 데이터 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> DefaultAnimInstanceClass;

};