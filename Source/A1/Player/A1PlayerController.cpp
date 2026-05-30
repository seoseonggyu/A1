// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(A1PlayerController)

DEFINE_LOG_CATEGORY(A1PlayerControllerLog);

AA1PlayerController::AA1PlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AA1PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// TODO: ÆÀ°ü·Ã
	//FDoRepLifetimeParams Params;
	//Params.bIsPushBased = true;

	//DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, TeamTag, Params);
}
