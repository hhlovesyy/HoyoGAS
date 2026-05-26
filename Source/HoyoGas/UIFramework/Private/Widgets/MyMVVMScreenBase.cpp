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
	UMyUIStoreSubsystem* ResolveScreenStoreSubsystem(const UUserWidget* Widget)
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

	void SetScreenWidgetViewModel(UUserWidget* Widget, UObject* InViewModel)
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
	return ResolveScreenStoreSubsystem(this);
}

UObject* UMyMVVMScreenBase::CreateViewModelInstance()
{
	return nullptr;
}

void UMyMVVMScreenBase::InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem)
{
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
	SetScreenWidgetViewModel(this, ViewModelObject);
}

void UMyMVVMScreenBase::DetachViewModelObject()
{
	SetScreenWidgetViewModel(this, nullptr);
	ViewModelObject = nullptr;
}
