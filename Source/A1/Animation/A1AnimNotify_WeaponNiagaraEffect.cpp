// #include "A1AnimNotify_WeaponNiagaraEffect.h"
//
// #include "Actors/A1EquipmentBase.h"
// #include "Character/LyraCharacter.h"
// #include "Item/Managers/A1EquipManagerComponent.h"
//
// #include UE_INLINE_GENERATED_CPP_BY_NAME(A1AnimNotify_WeaponNiagaraEffect)
//
// UA1AnimNotify_WeaponNiagaraEffect::UA1AnimNotify_WeaponNiagaraEffect()
// 	: Super()
// {
// #if WITH_EDITORONLY_DATA
// 	bShouldFireInEditor = false;
// #endif
// }
//
// void UA1AnimNotify_WeaponNiagaraEffect::Notify(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
// {
// 	USkeletalMeshComponent* WeaponMeshComponent = GetWeaponMeshComponent(MeshComponent);
// 	Super::Notify(WeaponMeshComponent ? WeaponMeshComponent : MeshComponent, Animation, EventReference);
// }
//
// USkeletalMeshComponent* UA1AnimNotify_WeaponNiagaraEffect::GetWeaponMeshComponent(USkeletalMeshComponent* MeshComponent) const
// {
// 	USkeletalMeshComponent* WeaponMeshComponent = nullptr;
// 	
// 	if (ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(MeshComponent->GetOwner()))
// 	{
// 		if (UA1EquipManagerComponent* EquipManager = LyraCharacter->FindComponentByClass<UA1EquipManagerComponent>())
// 		{
// 			if (AA1EquipmentBase* WeaponActor = EquipManager->GetEquippedActor(WeaponHandType))
// 			{
// 				WeaponMeshComponent = WeaponActor->MeshComponent;
// 			}
// 		}
// 	}
//
// 	return WeaponMeshComponent;
// }
