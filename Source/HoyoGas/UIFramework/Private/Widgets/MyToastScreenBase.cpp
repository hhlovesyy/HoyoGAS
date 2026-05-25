#include "Widgets/MyToastScreenBase.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/Widget.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMyToastScreenBase::UMyToastScreenBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PreferredLayer = EMyUILayer::Toast;
	bIsBackHandler = false;
	bSupportsActivationFocus = false;
	bAutoRestoreFocus = false;
}

void UMyToastScreenBase::SetPayload(const FMyUIPayload& InPayload)
{
	Super::SetPayload(InPayload);
	SetToastStackIndex(InPayload.IntValue);
	RefreshToastFromPayload();
}

void UMyToastScreenBase::SetToastStackIndex(int32 InStackIndex)
{
	ToastStackIndex = FMath::Max(0, InStackIndex);
	ApplyToastStackPosition();
}

void UMyToastScreenBase::NativeOnActivated()
{
	Super::NativeOnActivated();
	RefreshToastFromPayload();
	ApplyToastStackPosition();
	StartAutoCloseTimer();
}

void UMyToastScreenBase::NativeOnDeactivated()
{
	ClearAutoCloseTimer();
	Super::NativeOnDeactivated();
}

void UMyToastScreenBase::NativeDestruct()
{
	ClearAutoCloseTimer();
	Super::NativeDestruct();
}

void UMyToastScreenBase::RefreshToastFromPayload()
{
	if (!ToastMessageText)
	{
		return;
	}

	ToastMessageText->SetText(FText::FromString(CachedPayload.StringValue));
}

void UMyToastScreenBase::StartAutoCloseTimer()
{
	ClearAutoCloseTimer();

	if (AutoCloseDelay <= 0.0f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerDelegate CloseDelegate = FTimerDelegate::CreateWeakLambda(this,
		[this]()
		{
			if (IsActivated())
			{
				RequestClose();
			}
		});

	World->GetTimerManager().SetTimer(AutoCloseTimerHandle, CloseDelegate, AutoCloseDelay, false);
}

void UMyToastScreenBase::ClearAutoCloseTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
	}
}

void UMyToastScreenBase::ApplyToastStackPosition()
{
	UWidget* ToastSizeBox = GetWidgetFromName(TEXT("ToastSizeBox"));
	if (!ToastSizeBox)
	{
		return;
	}

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ToastSizeBox->Slot);
	if (!CanvasSlot)
	{
		return;
	}

	const FVector2D Position(-StackRightOffset, StackTopOffset + StackItemSpacing * ToastStackIndex);
	CanvasSlot->SetPosition(Position);
}
