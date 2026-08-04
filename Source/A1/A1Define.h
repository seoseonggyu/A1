#pragma once


UENUM(BlueprintType)
enum class EEquipmentSlotType : uint8
{
	Unarmed_LeftHand,
	Unarmed_RightHand,

	Primary_LeftHand,
	Primary_RightHand,
	Primary_TwoHand,
	
	Utility_Primary,
	Utility_Secondary,
	Utility_Tertiary,
	Utility_Quaternary,

	Helmet,
	Chest,
	Legs,
	Hands,
	Foot,

	Count	UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EArmorType : uint8
{
	Helmet,
	Chest,
	Legs,
	Hands,
	Foot,

	Count	UMETA(Hidden)
};