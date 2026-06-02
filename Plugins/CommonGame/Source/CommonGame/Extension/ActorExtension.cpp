// Copyright Epic Games, Inc. All Rights Reserved.

#include "Extension/ActorExtension.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ActorExtension)

bool FActorExtension::CanActivate(AActor* Owner) const
{
	for (const auto& ConditionStruct : Conditions)
	{
		if (const FExtensionCondition* Condition = ConditionStruct.GetPtr())
		{
			if (!Condition->IsSatisfied(Owner))
			{
				return false;
			}
		}
	}
	return true;
}

void FActorExtension::OnActivate(AActor* Owner)
{
	if (bActivated)
	{
		return;
	}
	
	bActivated = true;

	for (const auto& ExecuteStruct : Executes)
	{
		if (const FExtensionExecute* Execute = ExecuteStruct.GetPtr())
		{
			Execute->OnActivate(Owner);
		}
	}
}

void FActorExtension::OnDeactivate(AActor* Owner)
{
	if (!bActivated)
	{
		return;
	}
	bActivated = false;

	for (const auto& ExecuteStruct : Executes)
	{
		if (const FExtensionExecute* Execute = ExecuteStruct.GetPtr())
		{
			Execute->OnDeactivate(Owner);
		}
	}
}