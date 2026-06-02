// Copyright Epic Games, Inc. All Rights Reserved.

#include "Extension/ActorExtensionWorldSubsystem.h"
#include "TimerManager.h"
#include "Experience/ExperienceManagerComponent.h"
#include "Game/CommonCharacter.h"
#include "Game/CommonPlayerController.h"
#include "Game/CommonGameStateBase.h"
#include "Game/CommonGameModeBase.h"
#include "Game/CommonPlayerState.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/GameInstance.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ActorExtensionWorldSubsystem)

DEFINE_LOG_CATEGORY(ActorExtensionWorldSubsystemLog);

void UActorExtensionWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// GFCM에 Extension Handler 등록
	RegisterWithGameFrameworkComponentManager();

	// Experience 로드 대기
	WaitForExperienceLoadedCoroutine();
}

TCoroTask<void> UActorExtensionWorldSubsystem::WaitForExperienceLoadedCoroutine()
{
	co_await UExperienceManagerComponent::WaitForExperienceLoadedStaticCoroutine(GetWorld());
	bExperienceLoaded = true;
}

void UActorExtensionWorldSubsystem::Deinitialize()
{
	// Complete 상태의 모든 Actor에 대해 Deactivate 호출
	for (auto& Pair : CompleteActors)
	{
		AActor* Actor = Pair.Key.Get();
		if (!Actor)
		{
			continue;
		}

		for (FActorExtension& Extension : Pair.Value.Extensions)
		{
			if (Extension.IsActivated())
			{
				Extension.OnDeactivate(Actor);
			}
		}
	}

	// 모든 데이터 정리
	UncheckedActors.Empty();
	RegisterActors.Empty();
	CompleteActors.Empty();
	ClassExtensionCache.Empty();
	ExtensionHandles.Empty();

	Super::Deinitialize();
}

void UActorExtensionWorldSubsystem::Tick(float DeltaTime)
{
	//-------------------------------------------------------------------------
	// 1단계: Unchecked → Register (Extension 매핑 확인)
	//-------------------------------------------------------------------------
	for (int32 i = UncheckedActors.Num() - 1; i >= 0; --i)
	{
		AActor* Actor = UncheckedActors[i].Get();
		if (!Actor)
		{
			UncheckedActors.RemoveAtSwap(i);
			continue;
		}

		// Extension 매핑 확인 및 복사본 생성
		TArray<FActorExtension> Extensions;
		if (TryCreateExtensions(Actor, Extensions))
		{
			// Register로 이동
			RegisterActors.Add(FRegisterActorData{ Actor, MoveTemp(Extensions) });
		}

		UncheckedActors.RemoveAtSwap(i);
	}

	//-------------------------------------------------------------------------
	// 2단계: Register → Complete (조건 평가 및 활성화)
	//-------------------------------------------------------------------------
	for (int32 i = RegisterActors.Num() - 1; i >= 0; --i)
	{
		FRegisterActorData& Data = RegisterActors[i];
		AActor* Actor = Data.Actor.Get();
		if (!Actor)
		{
			RegisterActors.RemoveAtSwap(i);
			continue;
		}

		// 모든 Extension에 대해 조건 평가 및 활성화
		bool bAllActivated = true;
		for (FActorExtension& Extension : Data.Extensions)
		{
			if (!Extension.IsActivated())
			{
				if (Extension.CanActivate(Actor))
				{
					Extension.OnActivate(Actor);
				}
				else
				{
					bAllActivated = false;
				}
			}
		}

		// 모든 Extension 활성화 완료 시 Complete로 이동
		if (bAllActivated)
		{
			CompleteActors.Add(Actor, FCompleteActorData{ MoveTemp(Data.Extensions) });
			RegisterActors.RemoveAtSwap(i);
		}
	}
}

TStatId UActorExtensionWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UActorExtensionWorldSubsystem, STATGROUP_Tickables);
}

bool UActorExtensionWorldSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	// 게임과 PIE에서만 동작
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

bool UActorExtensionWorldSubsystem::IsTickable() const
{
	return ((UncheckedActors.Num() + RegisterActors.Num()) > 0) && bExperienceLoaded;
}

//-----------------------------------------------------------------------------
// Extension 등록 API
//-----------------------------------------------------------------------------

void UActorExtensionWorldSubsystem::RegisterExtensionForClass(UClass* TargetClass, const FActorExtension& Extension, bool bAddToLocallyControlled, bool bAddToSimulatedProxy)
{
	if (!TargetClass)
	{
		return;
	}

	// 대상 클래스와 모든 서브클래스에 Extension 등록 (O(1) 조회를 위한 캐시)
	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(TargetClass, DerivedClasses, true);
	DerivedClasses.Add(TargetClass);

	for (UClass* Class : DerivedClasses)
	{
		FClassExtensionMapping& Mapping = ClassExtensionCache.FindOrAdd(Class);
		Mapping.Entries.Add(FExtensionEntry{ Extension, bAddToLocallyControlled, bAddToSimulatedProxy });
	}
}

void UActorExtensionWorldSubsystem::UnregisterExtensionsForClass(UClass* TargetClass)
{
	if (!TargetClass)
	{
		return;
	}

	// 대상 클래스와 모든 서브클래스에서 Extension 제거
	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(TargetClass, DerivedClasses, true);
	DerivedClasses.Add(TargetClass);

	for (UClass* Class : DerivedClasses)
	{
		ClassExtensionCache.Remove(Class);
	}
}

//-----------------------------------------------------------------------------
// 조회 API
//-----------------------------------------------------------------------------

bool UActorExtensionWorldSubsystem::HasExtensionsForClass(const UClass* ActorClass) const
{
	if (!ActorClass)
	{
		return false;
	}

	// O(1) 조회 (서브클래스가 이미 캐시에 등록되어 있음)
	return ClassExtensionCache.Contains(ActorClass);
}

bool UActorExtensionWorldSubsystem::IsActorComplete(const AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	return CompleteActors.Contains(Actor);
}

//-----------------------------------------------------------------------------
// GFCM 통합
//-----------------------------------------------------------------------------

void UActorExtensionWorldSubsystem::RegisterWithGameFrameworkComponentManager()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UGameFrameworkComponentManager* GFCM = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance);
	if (!GFCM)
	{
		return;
	}

	// Common* 클래스들에 대해서만 Extension Handler 등록 (성능 최적화)
	const TArray TargetClasses = {
		ACommonCharacter::StaticClass(),
		ACommonPlayerController::StaticClass(),
		ACommonGameStateBase::StaticClass(),
		ACommonGameModeBase::StaticClass(),
		ACommonPlayerState::StaticClass()
	};

	auto Delegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtensionEvent);

	for (UClass* TargetClass : TargetClasses)
	{
		ExtensionHandles.Add(GFCM->AddExtensionHandler(TargetClass, Delegate));
	}
}

void UActorExtensionWorldSubsystem::HandleActorExtensionEvent(AActor* Actor, FName EventName)
{
	if (!Actor)
	{
		return;
	}

	if (EventName == UGameFrameworkComponentManager::NAME_GameActorReady)
	{
		UncheckedActors.Add(Actor);
	}
	else if (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
	{
		RemoveActor(Actor);
	}
}

//-----------------------------------------------------------------------------
// 상태 전이 처리
//-----------------------------------------------------------------------------

bool UActorExtensionWorldSubsystem::TryCreateExtensions(AActor* Actor, TArray<FActorExtension>& OutExtensions)
{
	if (!Actor)
	{
		return false;
	}

	// O(1) 조회 (서브클래스가 이미 캐시에 등록되어 있음)
	const FClassExtensionMapping* Mapping = ClassExtensionCache.Find(Actor->GetClass());
	if (!Mapping || Mapping->Entries.Num() == 0)
	{
		return false;
	}

	const bool bIsLocallyControlled = Actor->HasLocalNetOwner();
	const bool bIsSimulatedProxy = (Actor->GetLocalRole() == ROLE_SimulatedProxy);

	// Extension 복사본 생성 (Role에 따라 필터링)
	for (const FExtensionEntry& Entry : Mapping->Entries)
	{
		// LocallyControlled 필터링
		if (bIsLocallyControlled && !Entry.bAddToLocallyControlled)
		{
			continue;
		}

		// SimulatedProxy 필터링
		if (bIsSimulatedProxy && !Entry.bAddToSimulatedProxy)
		{
			continue;
		}

		// FActorExtension 추가 
		OutExtensions.Add(Entry.Extension);
	}

	return OutExtensions.Num() > 0;
}

void UActorExtensionWorldSubsystem::RemoveActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	// CompleteActors에서 제거 (가장 확률 높음, Deactivate 호출)
	if (FCompleteActorData* Data = CompleteActors.Find(Actor))
	{
		for (FActorExtension& Extension : Data->Extensions)
		{
			if (Extension.IsActivated())
			{
				Extension.OnDeactivate(Actor);
			}
		}
		
		CompleteActors.Remove(Actor);
		return;
	}

	// RegisterActors에서 제거
	for (int32 i = RegisterActors.Num() - 1; i >= 0; --i)
	{
		if (RegisterActors[i].Actor.Get() == Actor)
		{
			RegisterActors.RemoveAtSwap(i);
			return;
		}
	}

	// UncheckedActors에서 제거
	for (int32 i = UncheckedActors.Num() - 1; i >= 0; --i)
	{
		if (UncheckedActors[i].Get() == Actor)
		{
			UncheckedActors.RemoveAtSwap(i);
			return;
		}
	}
}
