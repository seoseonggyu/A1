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

	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Utility_Primary);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Utility_Secondary);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Utility_Tertiary);

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
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Sprint);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Interact);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_DropItem);

	//-----------------------------------------------------------------------------
	// Ability
	//-----------------------------------------------------------------------------
	
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_1);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_2);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_3);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Skill_1);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Skill_2);

	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Sprint_Check);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Sprint_Active);

	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Interact);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Interact_Scan);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Interact_Pickup);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Interact_Door);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Interact_Player);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Interact_Extraction);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Interact_LootContainer);

	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_DropItem);

	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Death);

	//-----------------------------------------------------------------------------
	// Status
	//-----------------------------------------------------------------------------

	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Attack);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Skill);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_RejectHitReact);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Sprint);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_StaminaRegen_Blocked);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Interacting);
	A1_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death);

	
	//-----------------------------------------------------------------------------
	// Gameplay.Event
	//-----------------------------------------------------------------------------
	
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Montage_Begin);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Montage_Tick);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Montage_End);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Montage_Move);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Trace);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Reset);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Stun);

	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Interact);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Interact_Attack);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Interact_Pickup);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Interact_Door);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Interact_Player);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Interact_Extraction);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Interact_LootContainer);

	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Death);

	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_DropItem);

	//-----------------------------------------------------------------------------
	// Cooldown.Skill
	//-----------------------------------------------------------------------------

	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Skill_1);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Skill_2);

	//-----------------------------------------------------------------------------
	// SetByCaller_BaseDamage
	//-----------------------------------------------------------------------------

	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_BaseDamage);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_StaminaCost);

	//-----------------------------------------------------------------------------
	// GameplayCue
	//-----------------------------------------------------------------------------

	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Character_DamageTaken);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Character_HitShake);
	A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Impact);

}