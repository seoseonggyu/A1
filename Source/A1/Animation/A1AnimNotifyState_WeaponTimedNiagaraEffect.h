// #pragma once
//
// #include "A1Define.h"
// #include "AnimNotifyState_TimedNiagaraEffect.h"
// #include "A1AnimNotifyState_WeaponTimedNiagaraEffect.generated.h"
//
// UCLASS(Blueprintable, meta=(DisplayName="Weapon Timed Niagara Effect"), MinimalAPI)
// class UA1AnimNotifyState_WeaponTimedNiagaraEffect : public UAnimNotifyState_TimedNiagaraEffect
// {
// 	GENERATED_BODY()
// 	
// public:
// 	UA1AnimNotifyState_WeaponTimedNiagaraEffect(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
// 	
// public:
// 	virtual void NotifyBegin(class USkeletalMeshComponent* MeshComponent, class UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
// 	virtual void NotifyEnd(class USkeletalMeshComponent* MeshComponent, class UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
//
// private:
// 	USkeletalMeshComponent* GetWeaponMeshComponent(USkeletalMeshComponent* CharacterMeshComponent) const;
// 	
// protected:
// 	UPROPERTY(EditAnywhere)
// 	EWeaponHandType WeaponHandType = EWeaponHandType::LeftHand;
// };
