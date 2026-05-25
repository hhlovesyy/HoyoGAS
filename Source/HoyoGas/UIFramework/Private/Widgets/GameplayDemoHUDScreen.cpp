#include "Widgets/GameplayDemoHUDScreen.h"

#include "Stores/InventoryUIStore.h"
#include "Stores/PlayerVitalsUIStore.h"
#include "Stores/ProgressionUIStore.h"
#include "Subsystems/MyUIStoreSubsystem.h"
#include "VM_HUD.h"
#include "Components/ProgressBar.h"
#include "Engine/World.h"

namespace
{
	constexpr float HealthTweenTickInterval = 1.0f / 60.0f;
	constexpr float HealthTweenSnapTolerance = 0.001f;
}

UGameplayDemoHUDScreen::UGameplayDemoHUDScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PreferredLayer = EMyUILayer::Game;
}

UObject* UGameplayDemoHUDScreen::CreateViewModelInstance()
{
	return NewObject<UVM_HUD>(this);
}

void UGameplayDemoHUDScreen::InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem)
{
	if (UVM_HUD* HUDViewModel = Cast<UVM_HUD>(ViewModel))
	{
		HUDViewModel->Initialize(
			StoreSubsystem ? StoreSubsystem->GetStore<UProgressionUIStore>() : nullptr,
			StoreSubsystem ? StoreSubsystem->GetStore<UInventoryUIStore>() : nullptr,
			StoreSubsystem ? StoreSubsystem->GetStore<UPlayerVitalsUIStore>() : nullptr);
	}
}

void UGameplayDemoHUDScreen::TeardownViewModel(UObject* ViewModel)
{
	if (UVM_HUD* HUDViewModel = Cast<UVM_HUD>(ViewModel))
	{
		HUDViewModel->Teardown();
	}
}

void UGameplayDemoHUDScreen::HandlePostViewModelAttached()
{
	UVM_HUD* HUDViewModel = Cast<UVM_HUD>(GetViewModelObject());
	if (!HUDViewModel)
	{
		return;
	}

	HealthPercentChangedHandle = HUDViewModel->AddFieldValueChangedDelegate(
		UVM_HUD::FFieldNotificationClassDescriptor::HealthPercent,
		INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateUObject(
			this,
			&UGameplayDemoHUDScreen::HandleHealthPercentChanged));

	ResetHealthTween(HUDViewModel->GetHealthPercent());
}

void UGameplayDemoHUDScreen::HandlePreViewModelDetached(UObject* ViewModel)
{
	StopHealthTweenTimer();

	if (UVM_HUD* HUDViewModel = Cast<UVM_HUD>(ViewModel))
	{
		if (HealthPercentChangedHandle.IsValid())
		{
			HUDViewModel->RemoveFieldValueChangedDelegate(
				UVM_HUD::FFieldNotificationClassDescriptor::HealthPercent,
				HealthPercentChangedHandle);
		}
	}

	HealthPercentChangedHandle.Reset();
	bHasHealthTweenState = false;
}

UWidget* UGameplayDemoHUDScreen::NativeGetDesiredFocusTarget() const
{
	return nullptr;
}

void UGameplayDemoHUDScreen::ResetHealthTween(float InHealthPercent)
{
	const float ClampedPercent = FMath::Clamp(InHealthPercent, 0.0f, 1.0f);

	bHasHealthTweenState = true;
	HealthTargetPercent = ClampedPercent;
	HealthFillPercent = ClampedPercent;
	HealthTrailPercent = ClampedPercent;
	HealthTrailDelayRemaining = 0.0f;

	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(HealthFillPercent);
	}

	if (HealthTrailProgressBar)
	{
		HealthTrailProgressBar->SetPercent(HealthTrailPercent);
	}
}

void UGameplayDemoHUDScreen::ApplyHealthPercent(float InHealthPercent)
{
	const float NewTargetPercent = FMath::Clamp(InHealthPercent, 0.0f, 1.0f);

	if (!bHasHealthTweenState)
	{
		ResetHealthTween(NewTargetPercent);
		return;
	}

	if (FMath::IsNearlyEqual(HealthTargetPercent, NewTargetPercent, HealthTweenSnapTolerance))
	{
		return;
	}

	HealthTargetPercent = NewTargetPercent;

	if (NewTargetPercent < HealthFillPercent)
	{
		HealthFillPercent = NewTargetPercent;
		HealthTrailDelayRemaining = HealthTrailDelay;

		if (HealthProgressBar)
		{
			HealthProgressBar->SetPercent(HealthFillPercent);
		}
	}
	else
	{
		HealthTrailPercent = NewTargetPercent;
		HealthTrailDelayRemaining = 0.0f;

		if (HealthTrailProgressBar)
		{
			HealthTrailProgressBar->SetPercent(HealthTrailPercent);
		}
	}

	StartHealthTweenTimer();
}

void UGameplayDemoHUDScreen::StartHealthTweenTimer()
{
	UWorld* World = GetWorld();
	if (!World || HealthTweenTimerHandle.IsValid())
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		HealthTweenTimerHandle,
		this,
		&UGameplayDemoHUDScreen::HandleHealthTweenTimer,
		HealthTweenTickInterval,
		true);
}

void UGameplayDemoHUDScreen::StopHealthTweenTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HealthTweenTimerHandle);
	}

	HealthTweenTimerHandle.Invalidate();
}

void UGameplayDemoHUDScreen::HandleHealthTweenTimer()
{
	UpdateHealthTween(HealthTweenTickInterval);
}

void UGameplayDemoHUDScreen::UpdateHealthTween(float InDeltaTime)
{
	bool bNeedsMoreTime = false;

	if (!FMath::IsNearlyEqual(HealthFillPercent, HealthTargetPercent, HealthTweenSnapTolerance))
	{
		HealthFillPercent = FMath::FInterpTo(HealthFillPercent, HealthTargetPercent, InDeltaTime, HealthIncreaseInterpSpeed);
		if (FMath::IsNearlyEqual(HealthFillPercent, HealthTargetPercent, HealthTweenSnapTolerance))
		{
			HealthFillPercent = HealthTargetPercent;
		}
		else
		{
			bNeedsMoreTime = true;
		}

		if (HealthProgressBar)
		{
			HealthProgressBar->SetPercent(HealthFillPercent);
		}
	}

	if (HealthTrailDelayRemaining > 0.0f)
	{
		HealthTrailDelayRemaining = FMath::Max(0.0f, HealthTrailDelayRemaining - InDeltaTime);
		bNeedsMoreTime = true;
	}
	else if (!FMath::IsNearlyEqual(HealthTrailPercent, HealthTargetPercent, HealthTweenSnapTolerance))
	{
		HealthTrailPercent = FMath::FInterpTo(HealthTrailPercent, HealthTargetPercent, InDeltaTime, HealthTrailDecreaseInterpSpeed);
		if (FMath::IsNearlyEqual(HealthTrailPercent, HealthTargetPercent, HealthTweenSnapTolerance))
		{
			HealthTrailPercent = HealthTargetPercent;
		}
		else
		{
			bNeedsMoreTime = true;
		}

		if (HealthTrailProgressBar)
		{
			HealthTrailProgressBar->SetPercent(HealthTrailPercent);
		}
	}

	if (!bNeedsMoreTime)
	{
		StopHealthTweenTimer();
	}
}

void UGameplayDemoHUDScreen::HandleHealthPercentChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId)
{
	(void)FieldId;

	if (const UVM_HUD* HUDViewModel = Cast<UVM_HUD>(Object))
	{
		ApplyHealthPercent(HUDViewModel->GetHealthPercent());
	}
}
