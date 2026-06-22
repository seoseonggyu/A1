// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/Fragment/EquipmentFragment.h"
#include "GameplayTagContainer.h"
#include "Extension/UIExtensionTypes.h"
#include "EquipmentFragment_UIExtension.generated.h"

class UUserWidget;

/**
 * UIExtensionPoint에 등록할 위젯 요청
 */
USTRUCT(BlueprintType)
struct COMMONUIEXTENSION_API FEquipmentUIExtensionEntry
{
	GENERATED_BODY()

public:
	/** 등록할 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AssetBundles = "Client"))
	TSoftClassPtr<UUserWidget> WidgetClass;

	/** 대상 ExtensionPoint 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (Categories = "UI.ExtensionPoint"))
	FGameplayTag ExtensionPointTag;
};

/**
 * 장착 시 UIExtensionPoint에 위젯을 등록하는 Fragment
 *
 * 장착 시 지정된 위젯들을 UIExtension 시스템에 등록하고, 해제 시 제거합니다.
 * 클라이언트에서만 동작합니다.
 */
USTRUCT(BlueprintType, DisplayName = "UI Extension")
struct COMMONUIEXTENSION_API FEquipmentFragment_UIExtension : public FEquipmentFragment
{
	GENERATED_BODY()

public:
	virtual void OnEquipped(UEquipmentInstance* Instance) override;
	virtual void OnUnequipped(UEquipmentInstance* Instance) override;

public:
	/** 등록할 UIExtension 목록 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (TitleProperty = "{ExtensionPointTag} -> {WidgetClass}"))
	TArray<FEquipmentUIExtensionEntry> Extensions;

private:
	/** 등록된 Extension 핸들 (런타임) */
	UPROPERTY()
	TArray<FUIExtensionHandle> ExtensionHandles;
};