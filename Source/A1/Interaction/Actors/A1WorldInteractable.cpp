#include "A1WorldInteractable.h"

#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "UI/Interaction/A1InteractionPromptWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1WorldInteractable)

DEFINE_LOG_CATEGORY(A1WorldInteractableLog);

AA1WorldInteractable::AA1WorldInteractable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	// 커서 상호작용 트레이스에 걸리도록 기본 프로파일 지정. (물리 차단이 필요하면 BP에서 조정)
	Mesh->SetCollisionProfileName(TEXT("A1Interactable"));

	// 상호작용 프롬프트("줍기" 등). 월드 스페이스로 대상 근처에 표시되며 기본은 숨김.
	// 콜리전은 스캔/커서 트레이스를 방해하지 않도록 끈다. Widget Class는 BP에서 지정.
	PromptWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidgetComponent"));
	PromptWidgetComponent->SetupAttachment(Mesh);
	PromptWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	PromptWidgetComponent->SetDrawAtDesiredSize(true);
	PromptWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PromptWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	PromptWidgetComponent->SetVisibility(false);
}

void AA1WorldInteractable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, bIsUsed, Params);
}

void AA1WorldInteractable::GetHighlightComponents(TArray<UPrimitiveComponent*>& OutComponents) const
{
	if (Mesh)
	{
		OutComponents.Add(Mesh);
	}
}

void AA1WorldInteractable::GatherInteractionOptions(const FA1InteractionQuery& Query, TArray<FA1InteractionOption>& OutOptions) const
{
	FA1InteractionOption Option;
	Option.Interactable = TScriptInterface<IA1Interactable>(const_cast<AA1WorldInteractable*>(this));
	Option.Title = InteractionTitle;
	Option.InteractionRange = InteractionRange;
	Option.InteractEventTag = InteractEventTag;
	Option.HighlightStencil = HighlightStencil;
	OutOptions.Add(Option);
}

bool AA1WorldInteractable::CanInteract(const FA1InteractionQuery& Query) const
{
	// 소모형이면 아직 사용되지 않았을 때만 가능.
	return bConsumeOnUse ? (bIsUsed == false) : true;
}

void AA1WorldInteractable::SetInteractionPromptVisible(bool bVisible)
{
	if (PromptWidgetComponent == nullptr)
	{
		return;
	}

	PromptWidgetComponent->SetVisibility(bVisible, true);

	if (bVisible)
	{
		if (UA1InteractionPromptWidget* PromptWidget = Cast<UA1InteractionPromptWidget>(PromptWidgetComponent->GetWidget()))
		{
			PromptWidget->SetPromptText(InteractionTitle);
		}
	}
}

void AA1WorldInteractable::OnInteractAuth(AActor* Interactor)
{
	if (HasAuthority() == false)
		return;

	if (bConsumeOnUse)
	{
		bIsUsed = true;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, bIsUsed, this);
	}

	UE_LOG(A1WorldInteractableLog, Log, TEXT("%s 상호작용됨 (by %s)"), *GetName(), *GetNameSafe(Interactor));
}

void AA1WorldInteractable::OnRep_bIsUsed()
{
	// 클라 연출 훅. 기본은 없음. (하위 클래스/BP에서 확장)
}
