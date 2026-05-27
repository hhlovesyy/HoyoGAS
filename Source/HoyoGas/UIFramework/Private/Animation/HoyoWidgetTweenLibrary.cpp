#include "Animation/HoyoWidgetTweenLibrary.h"

#include "Components/Widget.h"
#include "TimerManager.h"

void UHoyoWidgetTweenHandle::Start(
	UWidget* InTargetWidget,
	float InDurationSeconds,
	EHoyoWidgetTweenEasing InEasing,
	float InEaseExponent,
	TFunction<void(float)> InTickFunction,
	TFunction<void()> InFinishFunction)
{
	Cancel();

	TargetWidget = InTargetWidget;
	DurationSeconds = FMath::Max(0.0f, InDurationSeconds);
	Easing = InEasing;
	EaseExponent = FMath::Max(0.01f, InEaseExponent);
	TickFunction = MoveTemp(InTickFunction);
	FinishFunction = MoveTemp(InFinishFunction);

	UWorld* World = InTargetWidget ? InTargetWidget->GetWorld() : nullptr;
	if (!World || !TickFunction)
	{
		Finish(true);
		return;
	}

	if (DurationSeconds <= 0.0f)
	{
		TickFunction(1.0f);
		if (FinishFunction)
		{
			FinishFunction();
		}
		return;
	}

	bActive = true;
	StartTimeSeconds = World->GetTimeSeconds();
	AddToRoot();

	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &UHoyoWidgetTweenHandle::Tick);
	World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.016f, true);
	Tick();
}

void UHoyoWidgetTweenHandle::Cancel()
{
	if (bActive)
	{
		Finish(true);
	}
}

bool UHoyoWidgetTweenHandle::IsActive() const
{
	return bActive;
}

void UHoyoWidgetTweenHandle::Tick()
{
	UWidget* Widget = TargetWidget.Get();
	UWorld* World = Widget ? Widget->GetWorld() : nullptr;
	if (!Widget || !World)
	{
		Finish(true);
		return;
	}

	const float Alpha = FMath::Clamp(static_cast<float>((World->GetTimeSeconds() - StartTimeSeconds) / DurationSeconds), 0.0f, 1.0f);
	const float EasedAlpha = EvaluateEasing(Alpha);
	TickFunction(EasedAlpha);

	if (Alpha >= 1.0f)
	{
		if (FinishFunction)
		{
			FinishFunction();
		}
		Finish(false);
	}
}

void UHoyoWidgetTweenHandle::Finish(bool bCancelled)
{
	UWidget* Widget = TargetWidget.Get();
	if (UWorld* World = Widget ? Widget->GetWorld() : nullptr)
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}

	const bool bWasActive = bActive;
	bActive = false;
	TargetWidget = nullptr;
	TickFunction = nullptr;
	FinishFunction = nullptr;

	if (bWasActive)
	{
		if (bCancelled)
		{
			OnCancelled.Broadcast();
		}
		else
		{
			OnFinished.Broadcast();
		}
	}

	if (IsRooted())
	{
		RemoveFromRoot();
	}
}

float UHoyoWidgetTweenHandle::EvaluateEasing(float Alpha) const
{
	const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

	switch (Easing)
	{
	case EHoyoWidgetTweenEasing::Linear:
		return ClampedAlpha;

	case EHoyoWidgetTweenEasing::EaseIn:
		return FMath::InterpEaseIn(0.0f, 1.0f, ClampedAlpha, EaseExponent);

	case EHoyoWidgetTweenEasing::EaseInOut:
		return FMath::InterpEaseInOut(0.0f, 1.0f, ClampedAlpha, EaseExponent);

	case EHoyoWidgetTweenEasing::EaseOut:
	default:
		return FMath::InterpEaseOut(0.0f, 1.0f, ClampedAlpha, EaseExponent);
	}
}

UHoyoWidgetTweenHandle* UHoyoWidgetTweenLibrary::TweenRenderTranslation(
	UWidget* Widget,
	FVector2D From,
	FVector2D To,
	float DurationSeconds,
	EHoyoWidgetTweenEasing Easing,
	float EaseExponent)
{
	if (!Widget)
	{
		return nullptr;
	}

	UHoyoWidgetTweenHandle* Handle = NewObject<UHoyoWidgetTweenHandle>(Widget);
	TWeakObjectPtr<UWidget> WeakWidget = Widget;

	Handle->Start(
		Widget,
		DurationSeconds,
		Easing,
		EaseExponent,
		[WeakWidget, From, To](float Alpha)
		{
			if (UWidget* StrongWidget = WeakWidget.Get())
			{
				StrongWidget->SetRenderTranslation(FMath::Lerp(From, To, Alpha));
			}
		},
		[WeakWidget, To]()
		{
			if (UWidget* StrongWidget = WeakWidget.Get())
			{
				StrongWidget->SetRenderTranslation(To);
			}
		});

	return Handle;
}

UHoyoWidgetTweenHandle* UHoyoWidgetTweenLibrary::TweenRenderScale(
	UWidget* Widget,
	FVector2D From,
	FVector2D To,
	float DurationSeconds,
	EHoyoWidgetTweenEasing Easing,
	float EaseExponent)
{
	if (!Widget)
	{
		return nullptr;
	}

	UHoyoWidgetTweenHandle* Handle = NewObject<UHoyoWidgetTweenHandle>(Widget);
	TWeakObjectPtr<UWidget> WeakWidget = Widget;

	Handle->Start(
		Widget,
		DurationSeconds,
		Easing,
		EaseExponent,
		[WeakWidget, From, To](float Alpha)
		{
			if (UWidget* StrongWidget = WeakWidget.Get())
			{
				StrongWidget->SetRenderScale(FMath::Lerp(From, To, Alpha));
			}
		},
		[WeakWidget, To]()
		{
			if (UWidget* StrongWidget = WeakWidget.Get())
			{
				StrongWidget->SetRenderScale(To);
			}
		});

	return Handle;
}

UHoyoWidgetTweenHandle* UHoyoWidgetTweenLibrary::TweenRenderOpacity(
	UWidget* Widget,
	float From,
	float To,
	float DurationSeconds,
	EHoyoWidgetTweenEasing Easing,
	float EaseExponent)
{
	if (!Widget)
	{
		return nullptr;
	}

	UHoyoWidgetTweenHandle* Handle = NewObject<UHoyoWidgetTweenHandle>(Widget);
	TWeakObjectPtr<UWidget> WeakWidget = Widget;

	Handle->Start(
		Widget,
		DurationSeconds,
		Easing,
		EaseExponent,
		[WeakWidget, From, To](float Alpha)
		{
			if (UWidget* StrongWidget = WeakWidget.Get())
			{
				StrongWidget->SetRenderOpacity(FMath::Lerp(From, To, Alpha));
			}
		},
		[WeakWidget, To]()
		{
			if (UWidget* StrongWidget = WeakWidget.Get())
			{
				StrongWidget->SetRenderOpacity(To);
			}
		});

	return Handle;
}
