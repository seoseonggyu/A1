// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

namespace CommonGameTags
{
	//-----------------------------------------------------------------------------
	// Input.Native - 네이티브 입력 태그
	//-----------------------------------------------------------------------------

	/** 이동 입력 */
	COMMONGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Move);

	/** 시점 입력 */
	COMMONGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Look);

	//-----------------------------------------------------------------------------
	// Input.Ability - Ability 입력 태그
	//-----------------------------------------------------------------------------

	/** 점프 입력 */
	COMMONGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Jump);

	/** 걷기 입력 */
	COMMONGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Walk);

	/** 달리기 입력 */
	COMMONGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Sprint);

	//-----------------------------------------------------------------------------
	// Gameplay - 게임플레이 상태 태그
	//-----------------------------------------------------------------------------

	/** Ability 입력이 차단된 상태 (스턴, 메뉴 등) */
	COMMONGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Gameplay_AbilityInputBlocked);
}
