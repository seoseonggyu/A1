// Copyright Epic Games, Inc. All Rights Reserved.

#include "ItemFragment_TagStat.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemFragment_TagStat)

void FNetState_TagStat::ApplyToFragment(FItemFragment& Fragment) const
{
	FFragment_TagStat& TagStat = static_cast<FFragment_TagStat&>(Fragment);
	TagStat.Value = Value;
}
