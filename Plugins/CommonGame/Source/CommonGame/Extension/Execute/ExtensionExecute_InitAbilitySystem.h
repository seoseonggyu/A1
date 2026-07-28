// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Coro.h"
#include "Extension/Execute/ExtensionExecute.h"
#include "UObject/SoftObjectPtr.h"
#include "ExtensionExecute_InitAbilitySystem.generated.h"

class UCommonAbilitySystemComponent;
class UCommonAbilityTagRelationshipMapping;

/**
 * AbilitySystem 초기화를 담당하는 Execute
 *
 * PlayerState의 ASC에 InitAbilityActorInfo를 호출하고,
 * PlayerController의 OnPostProcessInput에 ASC의 ProcessAbilityInput을 바인딩합니다.
 * 태그 관계 매핑(TagRelationshipMapping)을 코루틴으로 비동기 로딩하여 ASC에 주입합니다.
 * Pawn이 아닌 Actor에 적용하면 동작하지 않습니다.
 */
USTRUCT(BlueprintType, meta = (DisplayName = "Init Ability System"))
struct COMMONGAME_API FExtensionExecute_InitAbilitySystem : public FExtensionExecute
{
	GENERATED_BODY()

public:
	virtual void OnActivate(AActor* Owner) const override;
	virtual void OnDeactivate(AActor* Owner) const override;

public:
	/** 어빌리티 태그 간 Block/Cancel/활성 조건 관계를 정의하는 매핑 (비동기 로딩 후 ASC에 주입) */
	UPROPERTY(EditAnywhere, Category = "Tag Relationship", meta = (AssetBundles = "All"))
	TSoftObjectPtr<UCommonAbilityTagRelationshipMapping> TagRelationshipMapping;

private:
	/** TagRelationshipMapping을 비동기 로딩하여 ASC에 설정한다 (Owner = ASC). */
	TCoroTask<void> LoadTagRelationshipMappingCoroutine(UCommonAbilitySystemComponent* ASC) const;

private:
	/** ProcessAbilityInput 델리게이트 핸들 (런타임 상태) */
	mutable FDelegateHandle ProcessInputDelegateHandle;
};
