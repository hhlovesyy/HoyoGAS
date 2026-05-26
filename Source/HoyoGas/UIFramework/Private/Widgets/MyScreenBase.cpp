#include "Widgets/MyScreenBase.h"

#include "Engine/LocalPlayer.h"
#include "Engine/EngineBaseTypes.h"
#include "GameFramework/PlayerController.h"
#include "Input/UIActionBindingHandle.h"
#include "CommonInputModeTypes.h"
#include "Subsystems/MyPlayerUISubsystem.h"

namespace
{
	const TCHAR* ScreenLayerToString(const EMyUILayer Layer)
	{
		switch (Layer)
		{
		case EMyUILayer::Game:
			return TEXT("Game");
		case EMyUILayer::Menu:
			return TEXT("Menu");
		case EMyUILayer::Modal:
			return TEXT("Modal");
		case EMyUILayer::Toast:
			return TEXT("Toast");
		default:
			return TEXT("Unknown");
		}
	}
}

UMyScreenBase::UMyScreenBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsBackHandler = true;
	bAutoActivate = false;
	bSupportsActivationFocus = true;
	bAutoRestoreFocus = true;
}

void UMyScreenBase::SetPayload(const FMyUIPayload& InPayload)
{
	CachedPayload = InPayload;
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] SetPayload Screen=%s Layer=%s ContextTag=%s StringValue=%s IntValue=%d"),
		*GetNameSafe(this),
		ScreenLayerToString(PreferredLayer),
		*CachedPayload.ContextTag.ToString(),
		*CachedPayload.StringValue,
		CachedPayload.IntValue);
	BP_OnPayloadSet(CachedPayload);
}

void UMyScreenBase::RequestClose()
{
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] RequestClose Screen=%s Tag=%s Layer=%s Activated=%s"),
		*GetNameSafe(this),
		*ScreenTag.ToString(),
		ScreenLayerToString(PreferredLayer),
		IsActivated() ? TEXT("true") : TEXT("false"));

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer)
	{
		if (APlayerController* OwningPlayer = GetOwningPlayer())
		{
			LocalPlayer = OwningPlayer->GetLocalPlayer();
		}
	}

	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyScreenBase::RequestClose failed because LocalPlayer is null."));
		return;
	}

	if (UMyPlayerUISubsystem* PlayerUISubsystem = ULocalPlayer::GetSubsystem<UMyPlayerUISubsystem>(LocalPlayer))
	{
		PlayerUISubsystem->CloseScreen(this);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("UMyScreenBase::RequestClose failed because UMyPlayerUISubsystem is unavailable."));
}

FName UMyScreenBase::GetScreenTag() const
{
	return ScreenTag;
}

EMyUILayer UMyScreenBase::GetPreferredLayer() const
{
	return PreferredLayer;
}

bool UMyScreenBase::CanBeClosed_Implementation() const
{
	return true;
}

TOptional<FUIInputConfig> UMyScreenBase::GetDesiredInputConfig() const
{
	if (PreferredLayer == EMyUILayer::Menu)
	{
		FUIInputConfig InputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture, EMouseLockMode::DoNotLock, false);
		InputConfig.bIgnoreMoveInput = true;
		InputConfig.bIgnoreLookInput = true;
		return InputConfig;
	}

	return TOptional<FUIInputConfig>();
}

UWidget* UMyScreenBase::NativeGetDesiredFocusTarget() const
{
	if (UWidget* DesiredFocusTarget = BP_GetDesiredFocusTarget())
	{
		UE_LOG(LogTemp, Display, TEXT("[UIFramework] FocusTarget Screen=%s Tag=%s Source=Blueprint Target=%s"),
			*GetNameSafe(this),
			*ScreenTag.ToString(),
			*GetNameSafe(DesiredFocusTarget));
		return DesiredFocusTarget;
	}

	UWidget* SuperFocusTarget = Super::NativeGetDesiredFocusTarget();
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] FocusTarget Screen=%s Tag=%s Source=Super Target=%s"),
		*GetNameSafe(this),
		*ScreenTag.ToString(),
		*GetNameSafe(SuperFocusTarget));
	return SuperFocusTarget;
}

bool UMyScreenBase::NativeOnHandleBackAction()
{
	const bool bCanBeClosed = CanBeClosed();
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] BackIntent Screen=%s Tag=%s Layer=%s CanBeClosed=%s"),
		*GetNameSafe(this),
		*ScreenTag.ToString(),
		ScreenLayerToString(PreferredLayer),
		bCanBeClosed ? TEXT("true") : TEXT("false"));

	if (bCanBeClosed)
	{
		RequestClose();
		return true;
	}

	return false;
}

void UMyScreenBase::ConfigureScreen(FName InScreenTag, EMyUILayer InPreferredLayer)
{
	ScreenTag = InScreenTag;
	PreferredLayer = InPreferredLayer;
}

void UMyScreenBase::NativeOnActivated()
{
	Super::NativeOnActivated();
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] Activated Screen=%s Tag=%s Layer=%s FocusTarget=%s"),
		*GetNameSafe(this),
		*ScreenTag.ToString(),
		ScreenLayerToString(PreferredLayer),
		*GetNameSafe(NativeGetDesiredFocusTarget()));
	BP_OnScreenActivated();
}

void UMyScreenBase::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] Deactivated Screen=%s Tag=%s Layer=%s"),
		*GetNameSafe(this),
		*ScreenTag.ToString(),
		ScreenLayerToString(PreferredLayer));
	BP_OnScreenDeactivated();
}
