#include "A1GameplayTags.h"

namespace A1GameplayTags
{

	//-----------------------------------------------------------------------------
	// Equipment.Slot
	//-----------------------------------------------------------------------------

	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Unarmed_LeftHand, "Equipment.Slot.Unarmed.LeftHand");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Unarmed_RightHand, "Equipment.Slot.Unarmed.RightHand");

	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Weapon, "Equipment.Slot.Weapon");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Shield, "Equipment.Slot.Shield");

	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Secondary_LeftHand, "Equipment.Slot.Secondary.LeftHand");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Secondary_RightHand, "Equipment.Slot.Secondary.RightHand");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Secondary_TwoHand, "Equipment.Slot.Secondary.TwoHand");

	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Utility_Primary, "Equipment.Slot.Utility.Primary");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Utility_Secondary, "Equipment.Slot.Utility.Secondary");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Utility_Tertiary, "Equipment.Slot.Utility.Tertiary");

	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Helmet, "Equipment.Slot.Helmet");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Chest, "Equipment.Slot.Chest");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Legs, "Equipment.Slot.Legs");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Hands, "Equipment.Slot.Hands");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Foot, "Equipment.Slot.Foot");

	//-----------------------------------------------------------------------------
	// QuickBar.Slot
	//-----------------------------------------------------------------------------

	UE_DEFINE_GAMEPLAY_TAG(QuickBar_Slot_Primary, "QuickBar.Slot.Primary");
	UE_DEFINE_GAMEPLAY_TAG(QuickBar_Slot_Secondary, "QuickBar.Slot.Secondary");
	UE_DEFINE_GAMEPLAY_TAG(QuickBar_Slot_Tertiary, "QuickBar.Slot.Tertiary");
	UE_DEFINE_GAMEPLAY_TAG(QuickBar_Slot_Quaternary, "QuickBar.Slot.Quaternary");

	//-----------------------------------------------------------------------------
	// Input.Ability
	//-----------------------------------------------------------------------------

	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Attack, "Input.Ability.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Attack_Skill_1, "Input.Ability.Attack.Skill.1");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Attack_Skill_2, "Input.Ability.Attack.Skill.2");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Widget_Inventory, "Input.Ability.Widget.Inventory");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_QuickBar_Primary, "Input.Ability.QuickBar.Primary");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_QuickBar_Secondary, "Input.Ability.QuickBar.Secondary");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_QuickBar_Tertiary, "Input.Ability.QuickBar.Tertiary");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_QuickBar_Quaternary, "Input.Ability.QuickBar.Quaternary");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Sprint, "Input.Ability.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Interact, "Input.Ability.Interact");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_DropItem, "Input.Ability.DropItem");

	//-----------------------------------------------------------------------------
	// Ability
	//-----------------------------------------------------------------------------
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack, "Ability.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_1, "Ability.Attack.1");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_2, "Ability.Attack.2");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_3, "Ability.Attack.3");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_Skill_1, "Ability.Attack.Skill.1");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_Skill_2, "Ability.Attack.Skill.2");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Sprint_Check, "Ability.Sprint.Check");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Sprint_Active, "Ability.Sprint.Active");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Interact, "Ability.Interact");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Interact_Scan, "Ability.Interact.Scan");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Interact_Pickup, "Ability.Interact.Pickup");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Interact_Door, "Ability.Interact.Door");

	UE_DEFINE_GAMEPLAY_TAG(Ability_DropItem, "Ability.DropItem");


	//-----------------------------------------------------------------------------
	// Status
	//-----------------------------------------------------------------------------
	
	UE_DEFINE_GAMEPLAY_TAG(Status_Attack, "Status.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Status_Skill, "Status.Skill");
	UE_DEFINE_GAMEPLAY_TAG(Status_RejectHitReact, "Status.RejectHitReact");
	UE_DEFINE_GAMEPLAY_TAG(Status_Sprint, "Status.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(Status_StaminaRegen_Blocked, "Status.StaminaRegen.Blocked");
	UE_DEFINE_GAMEPLAY_TAG(Status_Interacting, "Status.Interacting");
	
	
	//-----------------------------------------------------------------------------
	// Gameplay.Event
	//-----------------------------------------------------------------------------
	
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Montage_Begin, "GameplayEvent.Montage.Begin");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Montage_Tick, "GameplayEvent.Montage.Tick");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Montage_End, "GameplayEvent.Montage.End");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Montage_Move, "GameplayEvent.Montage.Move");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Trace, "GameplayEvent.Trace");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Reset, "GameplayEvent.Reset");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Stun, "GameplayEvent.Stun");

	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Interact, "GameplayEvent.Interact");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Interact_Attack, "GameplayEvent.Interact.Attack");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Interact_Pickup, "GameplayEvent.Interact.Pickup");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Interact_Door, "GameplayEvent.Interact.Door");
		
	//-----------------------------------------------------------------------------
	// Cooldown.Skill
	//-----------------------------------------------------------------------------

	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_1, "Cooldown.Skill.1");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_2, "Cooldown.Skill.2");

	//-----------------------------------------------------------------------------
	// SetByCaller_BaseDamage
	//-----------------------------------------------------------------------------

	/** 데미지 값 (SetByCaller) */
	A1_API UE_DEFINE_GAMEPLAY_TAG(SetByCaller_BaseDamage, "SetByCaller.BaseDamage");

}