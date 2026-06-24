// Copyright Epic Games, Inc. All Rights Reserved.

#include "Layout/CommonPrimaryGameLayout.h"
#include "Layout/CommonUIManagerSubsystem.h"
#include "CommonActivatableWidget.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "ViewModel/CommonViewModelBase.h"
#include "View/MVVMView.h"
#include "View/MVVMViewClass.h"
#include "Coro.h"
#include "Awaiters/Asset.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonPrimaryGameLayout)

DEFINE_LOG_CATEGORY(CommonPrimaryGameLayoutLog);

UCommonPrimaryGameLayout::UCommonPrimaryGameLayout(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

UCommonPrimaryGameLayout* UCommonPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject)
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		return nullptr;
	}

	APlayerController* PlayerController = GameInstance->GetPrimaryPlayerController(false);
	return GetPrimaryGameLayout(PlayerController);
}

UCommonPrimaryGameLayout* UCommonPrimaryGameLayout::GetPrimaryGameLayout(const APlayerController* PlayerController)
{
	return PlayerController ? GetPrimaryGameLayout(PlayerController->GetLocalPlayer()) : nullptr;
}

UCommonPrimaryGameLayout* UCommonPrimaryGameLayout::GetPrimaryGameLayout(const ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer)
	{
		if (const UGameInstance* GameInstance = LocalPlayer->GetGameInstance())
		{
			if (UCommonUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCommonUIManagerSubsystem>())
			{
				return UIManager->GetRootLayoutForPlayer(LocalPlayer);
			}
		}
	}
	
	return nullptr;
}

void UCommonPrimaryGameLayout::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* Container)
{
	if (!LayerTag.IsValid())
	{
		UE_LOG(CommonPrimaryGameLayoutLog, Warning, TEXT("레이어 등록 실패: 유효하지 않은 레이어 태그"));
		return;
	}

	if (!Container)
	{
		UE_LOG(CommonPrimaryGameLayoutLog, Warning, TEXT("레이어 등록 실패: Container가 nullptr입니다"));
		return;
	}

	if (Layers.Contains(LayerTag))
	{
		UE_LOG(CommonPrimaryGameLayoutLog, Warning, TEXT("레이어 '%s'는 이미 등록되어 있습니다"), *LayerTag.ToString());
		return;
	}

	Layers.Add(LayerTag, Container);
}

UCommonActivatableWidgetContainerBase* UCommonPrimaryGameLayout::GetLayerContainer(FGameplayTag LayerTag) const
{
	return Layers.FindRef(LayerTag);
}

UCommonActivatableWidget* UCommonPrimaryGameLayout::PushWidgetToLayerStack(FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (UCommonActivatableWidgetContainerBase* Container = GetLayerContainer(LayerTag))
	{
		UCommonActivatableWidget* Widget = Container->AddWidget<UCommonActivatableWidget>(WidgetClass);

		// Widget의 MVVM 바인딩에 필요한 ViewModel 자동 설정
		SetupViewModelsForWidget(Widget);
		SetupViewModelsForWidgetTress(Widget);
		
		return Widget;
	}

	UE_LOG(CommonPrimaryGameLayoutLog, Warning, TEXT("레이어 '%s'를 찾을 수 없습니다"), *LayerTag.ToString());
	return nullptr;
}

TCoroTask<void> UCommonPrimaryGameLayout::PushWidgetToLayerStackCoroutine(FGameplayTag LayerTag, TSoftClassPtr<UCommonActivatableWidget> WidgetClass)
{
	if (WidgetClass.IsNull())
	{
		co_return;
	}

	// 비동기 로딩 (이미 로드되어 있으면 바로 반환)
	if (TSubclassOf<UCommonActivatableWidget> LoadedClass = co_await Coro::Async::LoadClass(this, WidgetClass))
	{
		PushWidgetToLayerStack(LayerTag, LoadedClass);
	}
	else
	{
		UE_LOG(CommonPrimaryGameLayoutLog, Warning, TEXT("위젯 클래스 비동기 로딩 실패: %s"), *WidgetClass.ToString());
	}
}

void UCommonPrimaryGameLayout::K2_PushWidgetToLayerStackAsync(FGameplayTag LayerTag, TSoftClassPtr<UCommonActivatableWidget> WidgetClass)
{
	PushWidgetToLayerStackCoroutine(LayerTag, WidgetClass);
}

void UCommonPrimaryGameLayout::PopWidgetFromLayerStack(FGameplayTag LayerTag)
{
	if (UCommonActivatableWidgetContainerBase* Container = GetLayerContainer(LayerTag))
	{
		if (UCommonActivatableWidget* ActiveWidget = Container->GetActiveWidget())
		{
			ActiveWidget->DeactivateWidget();
		}
	}
}

void UCommonPrimaryGameLayout::ClearAllLayers()
{
	for (const auto& LayerKVP : Layers)
	{
		if (LayerKVP.Value)
		{
			LayerKVP.Value->ClearWidgets();
		}
	}
}

void UCommonPrimaryGameLayout::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	// 위젯이 어느 레이어에 있는지 모르므로 모든 레이어를 순회합니다
	for (const auto& LayerKVP : Layers)
	{
		LayerKVP.Value->RemoveWidget(*ActivatableWidget);
	}
}

UCommonViewModelBase* UCommonPrimaryGameLayout::GetOrCreateViewModelByClass(FName ViewModelName, TSubclassOf<UCommonViewModelBase> ViewModelClass)
{
	if (ViewModelName.IsNone() || !ViewModelClass)
	{
		return nullptr;
	}

	// 캐시에서 찾기
	if (TObjectPtr<UCommonViewModelBase>* Found = ViewModels.Find(ViewModelName))
	{
		return *Found;
	}

	// 새로 생성 (Outer = this로 Layout 생명주기에 맞춤)
	UCommonViewModelBase* NewViewModel = NewObject<UCommonViewModelBase>(this, ViewModelClass);
	ViewModels.Add(ViewModelName, NewViewModel);
	
	return NewViewModel;
}

UCommonViewModelBase* UCommonPrimaryGameLayout::GetViewModelByName(FName ViewModelName) const
{
	if (const TObjectPtr<UCommonViewModelBase>* Found = ViewModels.Find(ViewModelName))
	{
		return *Found;
	}
	return nullptr;
}

void UCommonPrimaryGameLayout::SetupViewModelsForWidget(const UCommonActivatableWidget* Widget)
{
	if (!Widget)
	{
		return;
	}

	UMVVMView* View = Widget->GetExtension<UMVVMView>();
	if (!View)
	{
		return;
	}

	const UMVVMViewClass* ViewClass = View->GetViewClass();
	if (!ViewClass)
	{
		return;
	}

	// Widget이 필요로 하는 ViewModel 목록 순회
	for (const FMVVMViewClass_Source& Source : ViewClass->GetSources())
	{
		// ViewModel 타입인 경우만 처리
		if (!Source.IsViewModel())
		{
			continue;
		}

		FName ViewModelName = Source.GetName();
		UClass* ViewModelClass = Source.GetSourceClass();

		// UCommonViewModelBase 서브클래스인지 확인
		if (!ViewModelClass || !ViewModelClass->IsChildOf(UCommonViewModelBase::StaticClass()))
		{
			continue;
		}

		// GetOrCreate로 ViewModel 획득
		if (UCommonViewModelBase* ViewModel = GetOrCreateViewModelByClass(ViewModelName, TSubclassOf<UCommonViewModelBase>(ViewModelClass)))
		{
			// Widget의 MVVMView에 ViewModel 설정
			View->SetViewModel(ViewModelName, ViewModel);
		}
	}
}

void UCommonPrimaryGameLayout::SetupViewModelsForWidgetTress(const UCommonActivatableWidget* Widget)
{
	if (!Widget || !Widget->WidgetTree)
	{
		return;
	}

	TArray<UWidget*> TressWidgets;
	Widget->WidgetTree->GetAllWidgets(TressWidgets);
	for (UWidget* treeWidget : TressWidgets)
	{
		if (UCommonActivatableWidget* Found = Cast<UCommonActivatableWidget>(treeWidget))
		{
			SetupViewModelsForWidget(Found);
		}
	}
}
