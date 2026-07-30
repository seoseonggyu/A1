// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

namespace A1GameplayTags
{
	//-----------------------------------------------------------------------------
	// Equipment.Slot
	//-----------------------------------------------------------------------------
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Unarmed_LeftHand);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Unarmed_RightHand);

	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Weapon);

	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Secondary_LeftHand);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Secondary_RightHand);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Secondary_TwoHand);

	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Utility_Primary);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Utility_Secondary);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Utility_Tertiary);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Utility_Quaternary);

	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Helmet);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Chest);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Legs);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Hands);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Foot);

	//-----------------------------------------------------------------------------
	// QuickBar.Slot
	//-----------------------------------------------------------------------------

	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(QuickBar_Slot_Primary);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(QuickBar_Slot_Secondary);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(QuickBar_Slot_Tertiary);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(QuickBar_Slot_Quaternary);


	//-----------------------------------------------------------------------------
	// Input.Ability
	//-----------------------------------------------------------------------------

	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Skill_1);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Skill_2);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Widget_Inventory);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_QuickBar_Primary);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_QuickBar_Secondary);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_QuickBar_Tertiary);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_QuickBar_Quaternary);
	
	//-----------------------------------------------------------------------------
	// Ability
	//-----------------------------------------------------------------------------
	
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_1);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_2);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_3);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Skill_1);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Skill_2);
	
	//-----------------------------------------------------------------------------
	// Status
	//-----------------------------------------------------------------------------
	
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Attack);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Skill);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_RejectHitReact);
	
	
	//-----------------------------------------------------------------------------
	// Gameplay.Event
	//-----------------------------------------------------------------------------
	
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Montage_Begin);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Montage_Tick);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Montage_End);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Trace);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Stun);
	
	//-----------------------------------------------------------------------------
	// SetByCaller_BaseDamage
	//-----------------------------------------------------------------------------

	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_BaseDamage);

}