#include "Widgets/MyMVVMScreenBase.h"

#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "INotifyFieldValueChanged.h"
#include "MVVMSubsystem.h"
#include "Subsystems/MyUIStoreSubsystem.h"
#include "View/MVVMView.h"

namespace
{
	//匿名的命名空间：把函数包在 namespace { } 里，等于给这些函数加了一把锁：“这些函数只在当前这一个 .cpp 文件里可见，外面任何人、任何文件都调不到它们，也绝对不会和外面的同名函数冲突。” 这等价于 C 语言里的 static 函数，是极其优秀的防御性编程习惯。
	UMyUIStoreSubsystem* ResolveStoreSubsystem(const UUserWidget* Widget)
	{
		if (!Widget)
		{
			return nullptr;
		}

		ULocalPlayer* LocalPlayer = Widget->GetOwningLocalPlayer();
		if (!LocalPlayer)
		{
			if (const APlayerController* OwningPlayer = Widget->GetOwningPlayer())
			{
				LocalPlayer = OwningPlayer->GetLocalPlayer();
			}
		}

		return LocalPlayer ? ULocalPlayer::GetSubsystem<UMyUIStoreSubsystem>(LocalPlayer) : nullptr;
	}

	void SetWidgetViewModel(UUserWidget* Widget, UObject* InViewModel)
	{
		if (!Widget)
		{
			return;
		}

		if (UMVVMView* View = UMVVMSubsystem::GetViewFromUserWidget(Widget))
		{
			if (!InViewModel)
			{
				// Keep the last assigned source until the view is rebound or destructed.
				// For standard single-view-model pages this avoids double-uninitialization
				// inside UMVVMView when the widget is later removed from the tree.
				return;
			}

			TScriptInterface<INotifyFieldValueChanged> ViewModelInterface;
			if (INotifyFieldValueChanged* NotifyObject = Cast<INotifyFieldValueChanged>(InViewModel))
			{
				ViewModelInterface.SetObject(InViewModel);
				ViewModelInterface.SetInterface(NotifyObject);
			}

			View->SetViewModelByClass(ViewModelInterface);
		}
	}
}

void UMyMVVMScreenBase::NativeOnActivated()
{
	if (!ViewModelObject)
	{
		ViewModelObject = CreateViewModelInstance();
	}

	if (!ViewModelObject)
	{
		return;
	}

	InitializeViewModel(ViewModelObject, GetUIStoreSubsystem());
	AttachViewModelObject(ViewModelObject);
	HandlePostViewModelAttached();
	Super::NativeOnActivated();
}

void UMyMVVMScreenBase::NativeOnDeactivated()
{
	if (ViewModelObject)
	{
		HandlePreViewModelDetached(ViewModelObject);
		TeardownViewModel(ViewModelObject);
		DetachViewModelObject();
		HandlePostViewModelDetached();
	}

	Super::NativeOnDeactivated();
}

UObject* UMyMVVMScreenBase::GetViewModelObject() const
{
	return ViewModelObject;
}

UMyUIStoreSubsystem* UMyMVVMScreenBase::GetUIStoreSubsystem() const
{
	return ResolveStoreSubsystem(this);
}

UObject* UMyMVVMScreenBase::CreateViewModelInstance()
{
	return nullptr;
}

void UMyMVVMScreenBase::InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem)
{
	//强行告诉编译器：“我知道我传了这个变量，我就是故意不用的，你别给我报错了！”
	(void)ViewModel;
	(void)StoreSubsystem;
}

void UMyMVVMScreenBase::TeardownViewModel(UObject* ViewModel)
{
	(void)ViewModel;
}

void UMyMVVMScreenBase::HandlePostViewModelAttached()
{
}

void UMyMVVMScreenBase::HandlePreViewModelDetached(UObject* ViewModel)
{
	(void)ViewModel;
}

void UMyMVVMScreenBase::HandlePostViewModelDetached()
{
}

void UMyMVVMScreenBase::AttachViewModelObject(UObject* InViewModel)
{
	ViewModelObject = InViewModel;
	SetWidgetViewModel(this, ViewModelObject);
}

void UMyMVVMScreenBase::DetachViewModelObject()
{
	SetWidgetViewModel(this, nullptr);
	ViewModelObject = nullptr;
}
