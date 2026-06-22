// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

namespace CommonUIExtensionTags
{
	//-----------------------------------------------------------------------------
	// UI.Layer - UI 레이어 태그
	//-----------------------------------------------------------------------------

	/** HUD, 크로스헤어 등 게임 플레이 UI */
	COMMONUIEXTENSION_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Game);

	/** ESC 메뉴, 인벤토리 등 게임 메뉴 */
	COMMONUIEXTENSION_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_GameMenu);

	/** 메인 메뉴, 설정 */
	COMMONUIEXTENSION_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Menu);

	/** 확인 다이얼로그, 팝업 */
	COMMONUIEXTENSION_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Modal);
}
