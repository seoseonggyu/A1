// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/CommonGameStateBase.h"
#include "Experience/ExperienceManagerComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonGameStateBase)

ACommonGameStateBase::ACommonGameStateBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ExperienceManagerComponent = CreateDefaultSubobject<UExperienceManagerComponent>(TEXT("ExperienceManagerComponent"));
}