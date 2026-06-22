// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/Fragment/EquipmentFragment_UIExtension.h"
#include "Equipment/EquipmentInstance.h"
#include "Extension/UIExtensionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EquipmentFragment_UIExtension)

void FEquipmentFragment_UIExtension::OnEquipped(UEquipmentInstance* Instance)
{
	// 클라이언트에서만 위젯을 등록합니다
	if (!Instance->IsLocallyControlled())
	{
		return;
	}

	UUIExtensionSubsystem* ExtensionSubsystem = Instance->GetWorld()->GetSubsystem<UUIExtensionSubsystem>();
	if (!ExtensionSubsystem)
	{
		return;
	}

	for (const FEquipmentUIExtensionEntry& Entry : Extensions)
	{
		if (TSubclassOf<UUserWidget> WidgetClass = Entry.WidgetClass.Get())
		{
			FUIExtensionHandle Handle = ExtensionSubsystem->RegisterExtensionAsWidget(Entry.ExtensionPointTag, WidgetClass);
			ExtensionHandles.Add(Handle);
		}
	}
}

void FEquipmentFragment_UIExtension::OnUnequipped(UEquipmentInstance* Instance)
{
	if (!Instance->IsLocallyControlled())
	{
		return;
	}

	UUIExtensionSubsystem* ExtensionSubsystem = Instance->GetWorld()->GetSubsystem<UUIExtensionSubsystem>();
	if (!ExtensionSubsystem)
	{
		return;
	}

	for (FUIExtensionHandle& Handle : ExtensionHandles)
	{
		ExtensionSubsystem->UnregisterExtension(Handle);
	}

	ExtensionHandles.Empty();
}