// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonGameTags.h"

namespace CommonGameTags
{
	//-----------------------------------------------------------------------------
	// Input.Native - 네이티브 입력 태그
	//-----------------------------------------------------------------------------

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Native_Move, "Input.Native.Move", "이동 입력");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Native_Look, "Input.Native.Look", "시점 입력");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Native_Confirm, "Input.Native.Confirm", "확정 입력 (ASC->LocalInputConfirm)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Native_Cancel, "Input.Native.Cancel", "취소 입력 (ASC->LocalInputCancel)");

	//-----------------------------------------------------------------------------
	// Input.Ability - Ability 입력 태그
	//-----------------------------------------------------------------------------

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Ability_Jump, "Input.Ability.Jump", "점프 입력");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Ability_Walk, "Input.Ability.Walk", "걷기 입력");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Ability_Sprint, "Input.Ability.Sprint", "달리기 입력");

	//-----------------------------------------------------------------------------
	// Gameplay - 게임플레이 상태 태그
	//-----------------------------------------------------------------------------

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gameplay_AbilityInputBlocked, "Gameplay.AbilityInputBlocked", "Ability 입력이 차단된 상태");
}
