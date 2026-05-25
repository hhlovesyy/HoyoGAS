#include "Widgets/MyDialogBase.h"

#include "Engine/EngineBaseTypes.h"
#include "Input/UIActionBindingHandle.h"
#include "CommonInputModeTypes.h"

UMyDialogBase::UMyDialogBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PreferredLayer = EMyUILayer::Modal;
	bIsModal = true;
}

void UMyDialogBase::SetupDialog(FMyConfirmDialogConfig InConfig)
{
	DialogConfig = InConfig;
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] SetupDialog Screen=%s Title=%s Message=%s Confirm=%s Cancel=%s"),
		*GetNameSafe(this),
		*DialogConfig.Title.ToString(),
		*DialogConfig.Message.ToString(),
		*DialogConfig.ConfirmText.ToString(),
		*DialogConfig.CancelText.ToString());
	BP_OnDialogSetup(DialogConfig);
}

void UMyDialogBase::SetResultCallback(FMyDialogResultDelegate InCallback)
{
	ResultCallback = InCallback;
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] DialogResultCallbackBound Screen=%s Bound=%s"),
		*GetNameSafe(this),
		ResultCallback.IsBound() ? TEXT("true") : TEXT("false"));
}

TOptional<FUIInputConfig> UMyDialogBase::GetDesiredInputConfig() const
{
	FUIInputConfig InputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture, EMouseLockMode::DoNotLock, false);
	InputConfig.bIgnoreMoveInput = true;
	InputConfig.bIgnoreLookInput = true;
	return InputConfig;
}

void UMyDialogBase::NotifyDialogResult(bool bConfirmed)
{
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] ClickIntent Dialog=%s Result=%s"),
		*GetNameSafe(this),
		bConfirmed ? TEXT("Confirm") : TEXT("Cancel"));

	if (ResultCallback.IsBound())
	{
		UE_LOG(LogTemp, Display, TEXT("[UIFramework] DialogResultCallbackExecute Dialog=%s Confirmed=%s"),
			*GetNameSafe(this),
			bConfirmed ? TEXT("true") : TEXT("false"));
		ResultCallback.Execute(bConfirmed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIFramework] DialogResultCallbackMissing Dialog=%s Confirmed=%s"),
			*GetNameSafe(this),
			bConfirmed ? TEXT("true") : TEXT("false"));
	}

	RequestClose();
}
