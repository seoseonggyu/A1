// #include "A1AnimNotify_SendWeaponEvent.h"
//
// #include "AbilitySystemBlueprintLibrary.h"
// #include "Actors/A1EquipmentBase.h"
// #include "Character/LyraCharacter.h"
// #include "Item/Managers/A1EquipManagerComponent.h"
//
// #include UE_INLINE_GENERATED_CPP_BY_NAME(A1AnimNotify_SendWeaponEvent)
//
// UA1AnimNotify_SendWeaponEvent::UA1AnimNotify_SendWeaponEvent(const FObjectInitializer& ObjectInitializer)
// 	: Super(ObjectInitializer)
// {
// #if WITH_EDITORONLY_DATA
// 	bShouldFireInEditor = false;
// #endif
// 	bIsNativeBranchingPoint = true;
// }
//
// void UA1AnimNotify_SendWeaponEvent::Notify(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
// {
// 	Super::Notify(MeshComponent, Animation, EventReference);
//
// 	if (ALyraCharacter* Character = Cast<ALyraCharacter>(MeshComponent->GetOwner()))
// 	{
// 		if (UA1EquipManagerComponent* EquipManager = Character->FindComponentByClass<UA1EquipManagerComponent>())
// 		{
// 			AA1EquipmentBase* WeaponActor = EquipManager->GetEquippedActor(WeaponHandType);
// 			if (WeaponActor && EventData.EventTag.IsValid())
// 			{
// 				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(WeaponActor, EventData.EventTag, EventData);
// 			}
// 		}
// 	}
// }
