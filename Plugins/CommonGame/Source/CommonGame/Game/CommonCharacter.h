// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularCharacter.h"
#include "AbilitySystemInterface.h"
#include "CommonCharacter.generated.h"

class UCommonCameraComponent;
class USkeletalMeshComponent;
class UEquipmentInstance;

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

	/**
	 * AnimLayer를 설정합니다.
	 * @param Requester 이 요청을 보낸 장비 인스턴스 (nullptr이면 무조건 적용). ResetAnimationToDefault의
	 *        소유자 검증과 짝을 이루므로, 장비 Fragment에서 호출할 때는 반드시 자기 자신을 넘겨야 한다.
	 */
	void SetAnimationData(TSubclassOf<UAnimInstance> AnimLayerClass, UEquipmentInstance* Requester = nullptr) const;

	/**
	 * AnimLayer를 기본값으로 되돌립니다.
	 *
	 * @param Requester 이 요청을 보낸 장비 인스턴스. 같은 리플리케이션 델타에서 메인 장비가 바뀔 때
	 *        Iris FastArray는 엔트리 간 콜백 순서를 보장하지 않으므로, 해제(Old) 콜백이 활성화(New)
	 *        콜백보다 "나중에" 도착할 수 있다. 이때 Requester가 현재 AnimLayer 소유자가 아니면(=이미
	 *        다른 장비가 새 AnimLayer를 적용했으면) 리셋을 무시해 방금 적용된 상태를 덮어쓰지 않는다.
	 *        nullptr이면 소유자 검증 없이 무조건 리셋한다.
	 */
	virtual void ResetAnimationToDefault(UEquipmentInstance* Requester = nullptr) const;


protected:
	/** Top-Down 카메라 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCommonCameraComponent> CameraComponent;

	/** 적용할 애니메이션 데이터 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> DefaultAnimInstanceClass;

private:
	/** 현재 AnimLayer를 적용한 장비 인스턴스 (동시 활성/비활성 콜백의 순서 뒤바뀜으로 인한 덮어쓰기 방지용) */
	mutable TWeakObjectPtr<UEquipmentInstance> CurrentAnimLayerOwner;
};