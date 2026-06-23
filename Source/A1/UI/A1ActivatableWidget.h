#pragma once


#include "CommonActivatableWidget.h"
#include "A1ActivatableWidget.generated.h"

class UCommonViewModelBase;

UCLASS()
class A1_API UA1ActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UA1ActivatableWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void NativeOnInitialized() override;
};