// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonUIExtensionTags.h"

namespace CommonUIExtensionTags
{
	//-----------------------------------------------------------------------------
	// UI.Layer - UI 레이어 태그
	//-----------------------------------------------------------------------------

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Game, "UI.Layer.Game", "HUD, 크로스헤어 등 게임 플레이 UI");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_GameMenu, "UI.Layer.GameMenu", "ESC 메뉴, 인벤토리 등 게임 메뉴");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Menu, "UI.Layer.Menu", "메인 메뉴, 설정");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Modal, "UI.Layer.Modal", "확인 다이얼로그, 팝업");
}
