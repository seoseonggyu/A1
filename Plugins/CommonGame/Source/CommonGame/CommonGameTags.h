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

	/** 확정 입력. ASC->LocalInputConfirm()으로 라우팅되어 UAbilityTask_WaitConfirmCancel의 OnConfirm을 트리거한다. */
	COMMONGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Confirm);

	/** 취소 입력. ASC->LocalInputCancel()으로 라우팅되어 UAbilityTask_WaitConfirmCancel의 OnCancel을 트리거한다. */
	COMMONGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Cancel);

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
