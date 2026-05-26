#include "Widgets/MyMVVMDialogBase.h"

#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "INotifyFieldValueChanged.h"
#include "MVVMSubsystem.h"
#include "Subsystems/MyUIStoreSubsystem.h"
#include "View/MVVMView.h"

namespace
{
	UMyUIStoreSubsystem* ResolveDialogStoreSubsystem(const UUserWidget* Widget)
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

	void SetDialogWidgetViewModel(UUserWidget* Widget, UObject* InViewModel)
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

void UMyMVVMDialogBase::NativeOnActivated()
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

void UMyMVVMDialogBase::NativeOnDeactivated()
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

UObject* UMyMVVMDialogBase::GetViewModelObject() const
{
	return ViewModelObject;
}

UMyUIStoreSubsystem* UMyMVVMDialogBase::GetUIStoreSubsystem() const
{
	return ResolveDialogStoreSubsystem(this);
}

UObject* UMyMVVMDialogBase::CreateViewModelInstance()
{
	return nullptr;
}

void UMyMVVMDialogBase::InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem)
{
	(void)ViewModel;
	(void)StoreSubsystem;
}

void UMyMVVMDialogBase::TeardownViewModel(UObject* ViewModel)
{
	(void)ViewModel;
}

void UMyMVVMDialogBase::HandlePostViewModelAttached()
{
}

void UMyMVVMDialogBase::HandlePreViewModelDetached(UObject* ViewModel)
{
	(void)ViewModel;
}

void UMyMVVMDialogBase::HandlePostViewModelDetached()
{
}

void UMyMVVMDialogBase::AttachViewModelObject(UObject* InViewModel)
{
	ViewModelObject = InViewModel;
	SetDialogWidgetViewModel(this, ViewModelObject);
}

void UMyMVVMDialogBase::DetachViewModelObject()
{
	SetDialogWidgetViewModel(this, nullptr);
	ViewModelObject = nullptr;
}
