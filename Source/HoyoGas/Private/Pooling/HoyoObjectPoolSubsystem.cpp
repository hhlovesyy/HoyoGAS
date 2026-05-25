#include "Pooling/HoyoObjectPoolSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void UHoyoObjectPoolSubsystem::Deinitialize()
{
	ClearUserWidgetPools();
	Super::Deinitialize();
}

UUserWidget* UHoyoObjectPoolSubsystem::AcquireUserWidget(TSubclassOf<UUserWidget> WidgetClass, UObject* OwningContext)
{
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ObjectPool] AcquireUserWidget failed because WidgetClass is null."));
		return nullptr;
	}

	FHoyoUserWidgetPool& Pool = FindOrAddWidgetPool(WidgetClass);
	while (!Pool.IdleWidgets.IsEmpty())
	{
		UUserWidget* Widget = Pool.IdleWidgets.Pop(EAllowShrinking::No);
		if (!IsValid(Widget))
		{
			continue;
		}

		Widget->SetIsEnabled(true);
		Widget->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogTemp, Display, TEXT("[ObjectPool] ReuseWidget Class=%s Widget=%s IdleRemaining=%d"),
			*GetNameSafe(*WidgetClass),
			*GetNameSafe(Widget),
			Pool.IdleWidgets.Num());
		return Widget;
	}

	UUserWidget* NewWidget = CreateUserWidget(WidgetClass, OwningContext);
	UE_LOG(LogTemp, Display, TEXT("[ObjectPool] CreateWidget Class=%s Widget=%s"),
		*GetNameSafe(*WidgetClass),
		*GetNameSafe(NewWidget));
	return NewWidget;
}

bool UHoyoObjectPoolSubsystem::ReleaseUserWidget(UUserWidget* Widget)
{
	if (!IsValid(Widget))
	{
		return false;
	}

	Widget->RemoveFromParent();
	Widget->SetIsEnabled(false);
	Widget->SetVisibility(ESlateVisibility::Collapsed);

	FHoyoUserWidgetPool& Pool = FindOrAddWidgetPool(Widget->GetClass());
	Pool.IdleWidgets.AddUnique(Widget);
	UE_LOG(LogTemp, Display, TEXT("[ObjectPool] ReleaseWidget Class=%s Widget=%s IdleCount=%d"),
		*GetNameSafe(Widget->GetClass()),
		*GetNameSafe(Widget),
		Pool.IdleWidgets.Num());
	return true;
}

void UHoyoObjectPoolSubsystem::TrimUserWidgetPool(TSubclassOf<UUserWidget> WidgetClass, int32 MaxIdleCount)
{
	if (!WidgetClass)
	{
		return;
	}

	FHoyoUserWidgetPool* Pool = FindWidgetPool(WidgetClass);
	if (!Pool)
	{
		return;
	}

	const int32 ClampedMaxIdleCount = FMath::Max(0, MaxIdleCount);
	while (Pool->IdleWidgets.Num() > ClampedMaxIdleCount)
	{
		Pool->IdleWidgets.RemoveAt(0, 1, EAllowShrinking::No);
	}
}

void UHoyoObjectPoolSubsystem::ClearUserWidgetPools()
{
	WidgetPools.Empty();
}

FHoyoUserWidgetPool& UHoyoObjectPoolSubsystem::FindOrAddWidgetPool(TSubclassOf<UUserWidget> WidgetClass)
{
	if (FHoyoUserWidgetPool* Pool = FindWidgetPool(WidgetClass))
	{
		return *Pool;
	}

	FHoyoUserWidgetPool& NewPool = WidgetPools.AddDefaulted_GetRef();
	NewPool.WidgetClass = WidgetClass;
	return NewPool;
}

FHoyoUserWidgetPool* UHoyoObjectPoolSubsystem::FindWidgetPool(TSubclassOf<UUserWidget> WidgetClass)
{
	for (FHoyoUserWidgetPool& Pool : WidgetPools)
	{
		if (Pool.WidgetClass == WidgetClass)
		{
			return &Pool;
		}
	}

	return nullptr;
}

UUserWidget* UHoyoObjectPoolSubsystem::CreateUserWidget(TSubclassOf<UUserWidget> WidgetClass, UObject* OwningContext) const
{
	if (!WidgetClass)
	{
		return nullptr;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(OwningContext))
	{
		return CreateWidget<UUserWidget>(PlayerController, WidgetClass);
	}

	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(OwningContext))
	{
		if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
		{
			return CreateWidget<UUserWidget>(PlayerController, WidgetClass);
		}

		if (UGameInstance* GameInstance = LocalPlayer->GetGameInstance())
		{
			return CreateWidget<UUserWidget>(GameInstance, WidgetClass);
		}
	}

	if (UGameInstance* GameInstance = Cast<UGameInstance>(OwningContext))
	{
		return CreateWidget<UUserWidget>(GameInstance, WidgetClass);
	}

	UWorld* World = OwningContext ? OwningContext->GetWorld() : GetWorld();
	if (World)
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			return CreateWidget<UUserWidget>(PlayerController, WidgetClass);
		}
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return CreateWidget<UUserWidget>(GameInstance, WidgetClass);
	}

	return nullptr;
}
