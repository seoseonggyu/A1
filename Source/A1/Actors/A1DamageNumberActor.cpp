#include "Actors/A1DamageNumberActor.h"

#include "Awaiters/Time.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "UI/Combat/A1DamageNumberWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1DamageNumberActor)

DEFINE_LOG_CATEGORY(A1DamageNumberActorLog);

AA1DamageNumberActor::AA1DamageNumberActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	DamageWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidgetComponent"));
	SetRootComponent(DamageWidgetComponent);
	DamageWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	DamageWidgetComponent->SetDrawAtDesiredSize(true);
	DamageWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AA1DamageNumberActor::InitializeDamageNumber(float DamageAmount)
{
	UA1DamageNumberWidget* DamageWidget = Cast<UA1DamageNumberWidget>(DamageWidgetComponent->GetWidget());
	if (!DamageWidget)
	{
		UE_LOG(A1DamageNumberActorLog, Warning, TEXT("[A1DamageNumberActor] DamageWidgetComponent에 UA1DamageNumberWidget이 없습니다"));
		Destroy();
		return;
	}

	const float TargetScale = DamageWidget->SetDamageAmount(DamageAmount);
	DamageWidget->SetRenderScale(FVector2D(StartScale));

	PlayPopAnimationCoroutine(TargetScale);
}

TCoroTask<void> AA1DamageNumberActor::PlayPopAnimationCoroutine(float TargetScale)
{
	UUserWidget* Widget = DamageWidgetComponent->GetWidget();
	if (!Widget)
	{
		Destroy();
		co_return;
	}

	const float OvershootScale = TargetScale * OvershootMultiplier;

	// 1) Start → Overshoot: 목표보다 살짝 더 크게 튀어오른다.
	for (int32 FrameIndex = 1; FrameIndex <= PopFrameCount; ++FrameIndex)
	{
		co_await Coro::Latent::NextTick(this);

		const float Alpha = static_cast<float>(FrameIndex) / static_cast<float>(PopFrameCount);
		Widget->SetRenderScale(FVector2D(FMath::Lerp(StartScale, OvershootScale, Alpha)));
	}

	// 2) Overshoot → Target: 눌러앉듯 목표 크기로 정착한다.
	for (int32 FrameIndex = 1; FrameIndex <= SettleFrameCount; ++FrameIndex)
	{
		co_await Coro::Latent::NextTick(this);

		const float Alpha = static_cast<float>(FrameIndex) / static_cast<float>(SettleFrameCount);
		Widget->SetRenderScale(FVector2D(FMath::Lerp(OvershootScale, TargetScale, Alpha)));
	}

	// 3) 유지 구간: 위로 떠오르며, 흔들림은 시간이 지날수록 잦아들고, 후반부에 페이드아웃한다.
	FVector2D CurrentShakeOffset = FVector2D::ZeroVector;

	const double HoldStartTime = FPlatformTime::Seconds();
	const double HoldEndTime = HoldStartTime + HoldDuration;
	while (FPlatformTime::Seconds() < HoldEndTime)
	{
		co_await Coro::Latent::NextTick(this);

		const float TimeRatio = HoldDuration > 0.f
			? FMath::Clamp(static_cast<float>((FPlatformTime::Seconds() - HoldStartTime) / HoldDuration), 0.f, 1.f)
			: 1.f;

		const float CurrentShakeAmplitude = FMath::Lerp(ShakeAmplitude, 0.f, TimeRatio);
		const float FloatOffsetY = -FMath::Lerp(0.f, FloatDistance, TimeRatio);

		// 매 프레임 새 랜덤 목표로 순간이동하지 않고, 목표를 향해 부드럽게 따라가게 해서 잔진동처럼 보이게 한다.
		const FVector2D ShakeTarget(FMath::RandRange(-CurrentShakeAmplitude, CurrentShakeAmplitude), FMath::RandRange(-CurrentShakeAmplitude, CurrentShakeAmplitude));
		CurrentShakeOffset = FMath::Lerp(CurrentShakeOffset, ShakeTarget, ShakeSmoothing);

		Widget->SetRenderTranslation(FVector2D(CurrentShakeOffset.X, CurrentShakeOffset.Y + FloatOffsetY));

		if (TimeRatio > FadeOutStartRatio)
		{
			const float FadeAlpha = 1.f - (TimeRatio - FadeOutStartRatio) / (1.f - FadeOutStartRatio);
			Widget->SetRenderOpacity(FMath::Clamp(FadeAlpha, 0.f, 1.f));
		}
	}

	Destroy();
}
