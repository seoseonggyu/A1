#include "CommonCameraComponent.h"
#include "CommonCameraMode.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonCameraComponent)


UCommonCameraComponent::UCommonCameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CameraModeStack = nullptr;
	FieldOfViewOffset = 0.0f;

	// 카메라 모드 결정은 항상 이 컴포넌트 자신이 담당한다. 외부 시스템은 SetBaseCameraMode/SetAbilityCameraMode로만 관여한다.
	DetermineCameraModeDelegate.BindUObject(this, &UCommonCameraComponent::DetermineCameraMode);
}

void UCommonCameraComponent::OnRegister()
{
	Super::OnRegister();

	if (!CameraModeStack)
	{
		CameraModeStack = NewObject<UCommonCameraModeStack>(this);
		check(CameraModeStack);
	}
}

void UCommonCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	check(CameraModeStack);

	UpdateCameraModes();

	FCommonCameraModeView CameraModeView;
	CameraModeStack->EvaluateStack(DeltaTime, CameraModeView);

	if (APawn* TargetPawn = Cast<APawn>(GetTargetActor()))
	{
		if (APlayerController* PC = TargetPawn->GetController<APlayerController>())
		{
			PC->SetControlRotation(CameraModeView.ControlRotation);
		}
	}

	CameraModeView.FieldOfView += FieldOfViewOffset;
	FieldOfViewOffset = 0.0f;

	SetWorldLocationAndRotation(CameraModeView.Location, CameraModeView.Rotation);
	FieldOfView = CameraModeView.FieldOfView;

	DesiredView.Location = CameraModeView.Location;
	DesiredView.Rotation = CameraModeView.Rotation;
	DesiredView.FOV = CameraModeView.FieldOfView;
	DesiredView.OrthoWidth = OrthoWidth;
	DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
	DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;
	DesiredView.AspectRatio = AspectRatio;
	DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
	DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
	DesiredView.ProjectionMode = ProjectionMode;

	DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
	if (PostProcessBlendWeight > 0.0f)
	{
		DesiredView.PostProcessSettings = PostProcessSettings;
	}


	if (IsXRHeadTrackedCamera())
	{
		Super::GetCameraView(DeltaTime, DesiredView);
	}
}

void UCommonCameraComponent::UpdateCameraModes()
{
	check(CameraModeStack);

	if (CameraModeStack->IsStackActivate())
	{
		if (DetermineCameraModeDelegate.IsBound())
		{
			if (const TSubclassOf<UCommonCameraMode> CameraMode = DetermineCameraModeDelegate.Execute())
			{
				CameraModeStack->PushCameraMode(CameraMode);
			}
		}
	}
}

void UCommonCameraComponent::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	check(CameraModeStack);
	CameraModeStack->GetBlendInfo(/*out*/ OutWeightOfTopLayer, /*out*/ OutTagOfTopLayer);
}

UCommonCameraMode* UCommonCameraComponent::GetTopCameraMode() const
{
	check(CameraModeStack);
	return CameraModeStack->GetTopCameraMode();
}

void UCommonCameraComponent::SetBaseCameraMode(TSubclassOf<UCommonCameraMode> NewBaseCameraMode)
{
	BaseCameraModeClass = NewBaseCameraMode;
}

void UCommonCameraComponent::SetAbilityCameraMode(TSubclassOf<UCommonCameraMode> NewCameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	AbilityCameraModeClass = NewCameraMode;
	AbilityCameraModeOwningHandle = OwningSpecHandle;
}

void UCommonCameraComponent::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	// 이미 다른 Ability가 새로 덮어썼다면(핸들 불일치) 그 오버라이드를 건드리지 않는다.
	if (AbilityCameraModeOwningHandle == OwningSpecHandle)
	{
		AbilityCameraModeClass = nullptr;
		AbilityCameraModeOwningHandle = FGameplayAbilitySpecHandle();
	}
}

TSubclassOf<UCommonCameraMode> UCommonCameraComponent::DetermineCameraMode() const
{
	if (AbilityCameraModeClass)
	{
		return AbilityCameraModeClass;
	}

	return BaseCameraModeClass;
}