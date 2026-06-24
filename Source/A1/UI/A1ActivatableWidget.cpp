#include "A1ActivatableWidget.h"
#include "ViewModel/CommonViewModelBase.h"
#include "View/MVVMView.h"
#include "View/MVVMViewClass.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1ActivatableWidget)

UA1ActivatableWidget::UA1ActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UA1ActivatableWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	//UMVVMView* View = GetExtension<UMVVMView>();
	//if (!View)
	//{
	//	return;
	//}

	//const UMVVMViewClass* ViewClass = View->GetViewClass();
	//if (!ViewClass)
	//{
	//	return;
	//}

	//for (const FMVVMViewClass_Source& Source : ViewClass->GetSources())
	//{
	//	// ViewModel 타입인 경우만 처리
	//	if (!Source.IsViewModel())
	//	{
	//		continue;
	//	}

	//	FName ViewModelName = Source.GetName();
	//	UClass* ViewModelClass = Source.GetSourceClass();

	//	// UCommonViewModelBase 서브클래스인지 확인
	//	if (!ViewModelClass || !ViewModelClass->IsChildOf(UCommonViewModelBase::StaticClass()))
	//	{
	//		continue;
	//	}

	//	if (ViewModelName.IsNone() || !ViewModelClass)
	//	{
	//		continue;
	//	}

	//	UCommonViewModelBase* ViewModel = NewObject<UCommonViewModelBase>(this, ViewModelClass);
	//	if (ViewModel)
	//	{
	//		// Widget의 MVVMView에 ViewModel 설정
	//		View->SetViewModel(ViewModelName, ViewModel);
	//	}
	//}

}
