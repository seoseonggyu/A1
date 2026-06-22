// Copyright Epic Games, Inc. All Rights Reserved.

#include "Action/GameFeatureAction_AddWidgets.h"
#include "Layout/CommonPrimaryGameLayout.h"
#include "Extension/UIExtensionSubsystem.h"
#include "Game/CommonHUD.h"
#include "CommonActivatableWidget.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddWidgets)

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "GameFeatures"

UGameFeatureAction_AddWidgets::UGameFeatureAction_AddWidgets()
{
	// 위젯은 클라이언트에서만 필요합니다
	bClientAction = true;
	bServerAction = false;
}

void UGameFeatureAction_AddWidgets::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	if (ContextData.Contains(Context))
	{
		Reset(Context);
	}
}

void UGameFeatureAction_AddWidgets::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if (GameInstance && World && World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			// HUD Actor가 생성될 때 위젯을 추가합니다
			TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentManager->AddExtensionHandler(
				ACommonHUD::StaticClass(),
				UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtension, ChangeContext));

			ActiveData.ComponentRequests.Add(ExtensionRequestHandle);
		}
	}
}

void UGameFeatureAction_AddWidgets::HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved || EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
	{
		RemoveWidgets(Actor, ChangeContext);
	}
	else if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded || EventName == UGameFrameworkComponentManager::NAME_GameActorReady)
	{
		AddWidgets(Actor, ChangeContext);
	}
}

void UGameFeatureAction_AddWidgets::AddWidgets(AActor* Actor, FGameFeatureStateChangeContext ChangeContext)
{
	ACommonHUD* HUD = CastChecked<ACommonHUD>(Actor);

	const APlayerController* PC = HUD->GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PC->Player);
	if (!LocalPlayer)
	{
		return;
	}

	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);
	FPerActorData& ActorData = ActiveData.ActorData.FindOrAdd(HUD);

	// 1. Layout 추가: PrimaryGameLayout의 레이어에 추가
	if (UCommonPrimaryGameLayout* RootLayout = UCommonPrimaryGameLayout::GetPrimaryGameLayout(LocalPlayer))
	{
		for (const FLayoutRequest& Entry : Layouts)
		{
			if (TSubclassOf<UCommonActivatableWidget> WidgetClass = Entry.LayoutClass.Get())
			{
				if (UCommonActivatableWidget* Widget = RootLayout->PushWidgetToLayerStack(Entry.LayerTag, WidgetClass))
				{
					ActorData.LayoutsAdded.Add(Widget);
				}
			}
		}
	}

	// 2. ExtensionPoint 추가: UIExtension 시스템으로 위젯 등록
	if (UUIExtensionSubsystem* ExtensionSubsystem = HUD->GetWorld()->GetSubsystem<UUIExtensionSubsystem>())
	{
		for (const FUIExtensionPointRequest& Entry : ExtensionPoints)
		{
			if (TSubclassOf<UUserWidget> WidgetClass = Entry.WidgetClass.Get())
			{
				FUIExtensionHandle Handle = ExtensionSubsystem->RegisterExtensionAsWidget(Entry.ExtensionPointTag, WidgetClass);
				ActorData.ExtensionHandles.Add(Handle);
			}
		}
	}
}

void UGameFeatureAction_AddWidgets::RemoveWidgets(AActor* Actor, FGameFeatureStateChangeContext ChangeContext)
{
	ACommonHUD* HUD = CastChecked<ACommonHUD>(Actor);

	FPerContextData* ActiveData = ContextData.Find(ChangeContext);
	if (!ActiveData)
	{
		return;
	}

	FPerActorData* ActorData = ActiveData->ActorData.Find(HUD);
	if (!ActorData)
	{
		return;
	}

	// Layout 제거
	for (TWeakObjectPtr<UCommonActivatableWidget>& AddedLayout : ActorData->LayoutsAdded)
	{
		if (AddedLayout.IsValid())
		{
			AddedLayout->DeactivateWidget();
		}
	}

	// ExtensionHandle 해제
	if (UUIExtensionSubsystem* ExtensionSubsystem = HUD->GetWorld()->GetSubsystem<UUIExtensionSubsystem>())
	{
		for (FUIExtensionHandle& Handle : ActorData->ExtensionHandles)
		{
			ExtensionSubsystem->UnregisterExtension(Handle);
		}
	}

	ActiveData->ActorData.Remove(HUD);
}

void UGameFeatureAction_AddWidgets::Reset(const FGameFeatureStateChangeContext& ChangeContext)
{
	FPerContextData* ActiveData = ContextData.Find(ChangeContext);
	if (!ActiveData)
	{
		return;
	}

	// ComponentRequests 해제
	ActiveData->ComponentRequests.Empty();

	// 모든 Actor의 데이터 정리
	// 참고: ExtensionHandle은 서브시스템이 Deinitialize될 때 자동으로 정리됩니다
	ActiveData->ActorData.Empty();

	ContextData.Remove(ChangeContext);
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddWidgets::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	// Layouts 검증
	{
		int32 EntryIndex = 0;
		for (const FLayoutRequest& Entry : Layouts)
		{
			if (Entry.LayoutClass.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("LayoutHasNullClass", "Layouts[{0}]: LayoutClass가 설정되지 않았습니다"), FText::AsNumber(EntryIndex)));
			}

			if (!Entry.LayerTag.IsValid())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("LayoutHasNoTag", "Layouts[{0}]: LayerTag가 설정되지 않았습니다"), FText::AsNumber(EntryIndex)));
			}

			++EntryIndex;
		}
	}

	// ExtensionPoints 검증
	{
		int32 EntryIndex = 0;
		for (const FUIExtensionPointRequest& Entry : ExtensionPoints)
		{
			if (Entry.WidgetClass.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("ExtensionPointHasNullClass", "ExtensionPoints[{0}]: WidgetClass가 설정되지 않았습니다"), FText::AsNumber(EntryIndex)));
			}

			if (!Entry.ExtensionPointTag.IsValid())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("ExtensionPointHasNoTag", "ExtensionPoints[{0}]: ExtensionPointTag가 설정되지 않았습니다"), FText::AsNumber(EntryIndex)));
			}

			++EntryIndex;
		}
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE