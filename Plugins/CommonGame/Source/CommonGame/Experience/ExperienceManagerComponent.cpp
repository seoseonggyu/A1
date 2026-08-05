// Copyright Epic Games, Inc. All Rights Reserved.

#include "ExperienceManagerComponent.h"
#include "ExperienceDefinition.h"
#include "Awaiters/Asset.h"
#include "Awaiters/Combinator.h"
#include "Awaiters/Delegate.h"
#include "Awaiters/Time.h"
#include "DeveloperStatics.h"
#include "GameFeaturesSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceManagerComponent)

DEFINE_LOG_CATEGORY(ExperienceManagerLog);


UExperienceManagerComponent::UExperienceManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UExperienceManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, CurrentExperienceId, Params);
}

void UExperienceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	DeactivateExperience();
}

void UExperienceManagerComponent::SetCurrentExperienceAuth(FPrimaryAssetId ExperienceId)
{
	check(GetOwner()->HasAuthority());
	check(LoadState == EExperienceLoadState::Unloaded);

	UE_LOG(ExperienceManagerLog, Log, TEXT("Experience ����: %s"), *ExperienceId.ToString());

	CurrentExperienceId = ExperienceId;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, CurrentExperienceId, this);

	LoadExperienceCoroutine();
}

const UExperienceDefinition* UExperienceManagerComponent::GetCurrentExperienceChecked() const
{
	check(LoadState == EExperienceLoadState::Loaded);
	check(CurrentExperience != nullptr);
	return CurrentExperience;
}


void UExperienceManagerComponent::OnRep_CurrentExperienceId()
{
	LoadExperienceCoroutine();
}


TCoroTask<void> UExperienceManagerComponent::LoadExperienceCoroutine()
{
	check(LoadState == EExperienceLoadState::Unloaded || LoadState == EExperienceLoadState::Deactivating);
	check(CurrentExperienceId.IsValid());

	// ���� ƽ���� ��� (��������Ʈ �켱���� ����) // Chef: ���� �Ҹ���?
	co_await Coro::Latent::NextTick(this);

	const TCHAR* NetRole = FDeveloperStatics::GetNetRoleString(GetOwner());

	LoadState = EExperienceLoadState::Loading;

	UE_LOG(ExperienceManagerLog, Log, TEXT("%s Experience ���� �ε� ����: %s"), NetRole, *CurrentExperienceId.ToString());

	CurrentExperience = co_await Coro::Async::LoadPrimaryAsset<UExperienceDefinition>(this, CurrentExperienceId, {});

	if (!CurrentExperience)
	{
		UE_LOG(ExperienceManagerLog, Error, TEXT("%s Experience ���� �ε� ����: %s"), NetRole, *CurrentExperienceId.ToString());
		co_return;
	}

	UE_LOG(ExperienceManagerLog, Log, TEXT("%s Experience ���� �ε� �Ϸ�: %s"), NetRole, *CurrentExperience->GetName());
	
	LoadState = EExperienceLoadState::LoadingGameFeatures;
	GameFeaturePluginURLs.Reset();

	if (CurrentExperience->GameFeaturesToEnable.Num() > 0)
	{
		UGameFeaturesSubsystem& GFS = UGameFeaturesSubsystem::Get();

		UE_LOG(ExperienceManagerLog, Log, TEXT("%s GameFeature ���� �ε� ����: %d��"), NetRole, CurrentExperience->GameFeaturesToEnable.Num());

		TArray<TCoroTask<void>> FeatureTasks;
		for (const FPrimaryAssetId& GameFeatureId : CurrentExperience->GameFeaturesToEnable)
		{
			const FString PluginName = GameFeatureId.PrimaryAssetName.ToString();

			FString PluginURL;
			if (!GFS.GetPluginURLByName(PluginName, PluginURL))
			{
				UE_LOG(ExperienceManagerLog, Error, TEXT("%s GameFeature �÷������� ã�� �� �����ϴ�: %s"), NetRole, *PluginName);
				continue;
			}

			GameFeaturePluginURLs.Add(PluginURL);
			FeatureTasks.Add(LoadGameFeatureCoroutine(PluginURL));
		}

		co_await Coro::Async::WhenAll(this, MoveTemp(FeatureTasks));
	}

	LoadState = EExperienceLoadState::Loaded;

	UE_LOG(ExperienceManagerLog, Log, TEXT("%s Experience �ε� �Ϸ�: %s"), NetRole, *CurrentExperience->GetName());


	OnExperienceLoaded_High.Broadcast(CurrentExperience);
	OnExperienceLoaded_High.Clear();

	OnExperienceLoaded_Normal.Broadcast(CurrentExperience);
	OnExperienceLoaded_Normal.Clear();

	OnExperienceLoaded_Low.Broadcast(CurrentExperience);
	OnExperienceLoaded_Low.Clear();
}

TCoroTask<void> UExperienceManagerComponent::LoadGameFeatureCoroutine(FString PluginURL) const
{
	const TCHAR* NetRole = FDeveloperStatics::GetNetRoleString(GetOwner());
	const bool bSuccess = co_await Coro::Async::LoadGameFeature(GetOwner(), PluginURL);

	if (!bSuccess)
	{
		UE_LOG(ExperienceManagerLog, Error, TEXT("%s GameFeature �ε� ����: %s"), NetRole, *PluginURL);
	}
	else
	{
		UE_LOG(ExperienceManagerLog, Log, TEXT("%s GameFeature �ε� �Ϸ�: %s"), NetRole, *PluginURL);
	}
}

void UExperienceManagerComponent::DeactivateExperience()
{
	if (LoadState == EExperienceLoadState::Unloaded || LoadState == EExperienceLoadState::Deactivating)
	{
		return;
	}

	LoadState = EExperienceLoadState::Deactivating;

	UGameFeaturesSubsystem& GFS = UGameFeaturesSubsystem::Get();

	for (const FString& PluginURL : GameFeaturePluginURLs)
	{
		UE_LOG(ExperienceManagerLog, Log, TEXT("GameFeature ��Ȱ��ȭ: %s"), *PluginURL);
		GFS.DeactivateGameFeaturePlugin(PluginURL);
	}

	CurrentExperience = nullptr;
	GameFeaturePluginURLs.Reset();
	LoadState = EExperienceLoadState::Unloaded;
}

//-----------------------------------------------------------------------------
// Experience ��� �ڷ�ƾ (Static)
//-----------------------------------------------------------------------------


TCoroTask<const UExperienceDefinition*> UExperienceManagerComponent::WaitForExperienceLoaded_HighStaticCoroutine(UObject* WorldContextObject)
{
	co_return co_await WaitForExperienceLoadedInternalCoroutine(WorldContextObject, 0);
}

TCoroTask<const UExperienceDefinition*> UExperienceManagerComponent::WaitForExperienceLoadedStaticCoroutine(UObject* WorldContextObject)
{
	co_return co_await WaitForExperienceLoadedInternalCoroutine(WorldContextObject, 1);
}

TCoroTask<const UExperienceDefinition*> UExperienceManagerComponent::WaitForExperienceLoaded_LowStaticCoroutine(UObject* WorldContextObject)
{
	co_return co_await WaitForExperienceLoadedInternalCoroutine(WorldContextObject, 2);
}

TCoroTask<const UExperienceDefinition*> UExperienceManagerComponent::WaitForExperienceLoadedInternalCoroutine(UObject* WorldContextObject, int32 Priority)
{
	if (!WorldContextObject)
	{
		co_return nullptr;
	}

	// GameState���� ExperienceManagerComponent ã��
	UExperienceManagerComponent* Manager = nullptr;
	while (!Manager)
	{
		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
		if (!World)
		{
			co_return nullptr;
		}

		if (AGameStateBase* GameState = World->GetGameState())
		{
			Manager = GameState->FindComponentByClass<UExperienceManagerComponent>();
		}

		if (!Manager)
		{
			co_await Coro::Latent::NextTick(WorldContextObject);
		}
	}

	if (Manager->IsExperienceLoaded())
	{
		co_return Manager->GetCurrentExperienceChecked();
	}

	// �켱������ ��������Ʈ ���
	switch (Priority)
	{
	case 0:
		co_return co_await Coro::Async::WaitForDelegate(Manager, Manager->OnExperienceLoaded_High);
	case 1:
		co_return co_await Coro::Async::WaitForDelegate(Manager, Manager->OnExperienceLoaded_Normal);
	default:
		co_return co_await Coro::Async::WaitForDelegate(Manager, Manager->OnExperienceLoaded_Low);
	}
}