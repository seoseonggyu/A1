// #include "A1AnimNotifyState_PlayWeaponSound.h"
//
// #include "Actors/A1EquipmentBase.h"
// #include "Character/LyraCharacter.h"
// #include "Components/AudioComponent.h"
// #include "Item/A1ItemInstance.h"
// #include "Item/Fragments/A1ItemFragment_Equipable_Weapon.h"
// #include "Item/Managers/A1EquipManagerComponent.h"
// #include "Kismet/GameplayStatics.h"
//
// #include UE_INLINE_GENERATED_CPP_BY_NAME(A1AnimNotifyState_PlayWeaponSound)
//
// UA1AnimNotifyState_PlayWeaponSound::UA1AnimNotifyState_PlayWeaponSound(const FObjectInitializer& ObjectInitializer)
// 	: Super(ObjectInitializer)
// {
//     
// }
//
// void UA1AnimNotifyState_PlayWeaponSound::NotifyBegin(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
// {
// 	Super::NotifyBegin(MeshComponent, Animation, TotalDuration, EventReference);
//
// 	if (WeaponHandType == EWeaponHandType::Count || WeaponSoundType == EWeaponSoundType::None)
// 		return;
//
// 	ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(MeshComponent->GetOwner());
// 	if (LyraCharacter == nullptr)
// 		return;
// 	
// 	UA1EquipManagerComponent* EquipManager = LyraCharacter->FindComponentByClass<UA1EquipManagerComponent>();
// 	if (EquipManager == nullptr)
// 		return;
// 	
// 	UA1ItemInstance* ItemInstance = EquipManager->GetEquippedItemInstance(WeaponHandType);
// 	if (ItemInstance == nullptr)
// 		return;
//
// 	AA1EquipmentBase* WeaponActor = EquipManager->GetEquippedActor(WeaponHandType);
// 	if (WeaponActor == nullptr)
// 		return;
// 	
// 	const UA1ItemFragment_Equipable_Weapon* WeaponFragment = ItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable_Weapon>();
// 	if (WeaponFragment == nullptr)
// 		return;
//
// 	USoundBase* SelectedSound = nullptr;
// 	
// 	switch (WeaponSoundType)
// 	{
// 	case EWeaponSoundType::Swing:	SelectedSound = WeaponFragment->AttackSwingSound;	break;
// 	case EWeaponSoundType::Custom:	SelectedSound = CustomSound;						break;
// 	}
//
// 	if (SelectedSound)
// 	{
// 		AudioComponent = UGameplayStatics::SpawnSoundAttached(SelectedSound, WeaponActor->MeshComponent, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true, 1.f, 1.f, 0.f);
// 	}
// }
//
// void UA1AnimNotifyState_PlayWeaponSound::NotifyEnd(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
// {
// 	if (AudioComponent)
// 	{
// 		AudioComponent->DestroyComponent();
// 	}
// 	
// 	Super::NotifyEnd(MeshComponent, Animation, EventReference);
// }
