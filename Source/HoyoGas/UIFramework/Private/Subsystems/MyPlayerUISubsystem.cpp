#include "Subsystems/MyPlayerUISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "Components/Widget.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Input/UIActionBindingHandle.h"
#include "CommonInputModeTypes.h"
#include "Pooling/HoyoObjectPoolSubsystem.h"
#include "Subsystems/MyUIRegistrySubsystem.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Widgets/MyDialogBase.h"
#include "Widgets/MyScreenBase.h"
#include "Widgets/MyToastScreenBase.h"
#include "Widgets/MyUIRootLayout.h"
#include "TimerManager.h"

namespace
{
	const TCHAR* LexToString(const EMyUILayer Layer)
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

void UMyPlayerUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UMyPlayerUISubsystem::Deinitialize()
{
	for (TPair<EMyUILayer, TArray<TObjectPtr<UMyScreenBase>>>& Pair : LayerScreens)
	{
		for (UMyScreenBase* Screen : Pair.Value)
		{
			if (!IsValid(Screen))
			{
				continue;
			}

			if (IsCommonStackLayer(Screen->GetPreferredLayer()))
			{
				if (UCommonActivatableWidgetStack* Stack = GetCommonStackForLayer(Screen->GetPreferredLayer()))
				{
					Stack->RemoveWidget(*Screen);
				}
			}
			else
			{
				if (Screen->IsActivated())
				{
					Screen->DeactivateWidget();
				}

				if (IsValid(RootLayout))
				{
					RootLayout->RemoveScreen(Screen);
				}
				else
				{
					Screen->RemoveFromParent();
				}
			}
		}
	}

	LayerScreens.Empty();
	ScreenInstancesByTag.Empty();
	ActiveToastScreens.Empty();

	if (IsValid(RootLayout))
	{
		RootLayout->RemoveFromParent();
		RootLayout = nullptr;
	}

	Super::Deinitialize();
}

bool UMyPlayerUISubsystem::EnsureRootLayout()
{
	if (IsValid(RootLayout))
	{
		return true;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::EnsureRootLayout failed because LocalPlayer is null."));
		return false;
	}

	UGameInstance* GameInstance = LocalPlayer->GetGameInstance();
	APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld());

	TSubclassOf<UMyUIRootLayout> ResolvedRootLayoutClass = ResolveRootLayoutClass();
	if (!ResolvedRootLayoutClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::EnsureRootLayout failed because RootLayoutClass is null."));
		return false;
	}

	if (PlayerController)
	{
		RootLayout = CreateWidget<UMyUIRootLayout>(PlayerController, ResolvedRootLayoutClass);
	}
	else if (GameInstance)
	{
		RootLayout = CreateWidget<UMyUIRootLayout>(GameInstance, ResolvedRootLayoutClass);
	}

	if (!IsValid(RootLayout))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::EnsureRootLayout failed because CreateWidget returned null."));
		return false;
	}

	if (!RootLayout->AddToPlayerScreen(0))
	{
		RootLayout->AddToViewport(0);
	}

	UE_LOG(LogTemp, Display, TEXT("[UIFramework] RootLayoutReady Layout=%s MenuStack=%s ModalStack=%s"),
		*GetNameSafe(RootLayout),
		*GetNameSafe(RootLayout->GetMenuStack()),
		*GetNameSafe(RootLayout->GetModalStack()));
	BindCommonStackDebugHooks();
	return true;
}

UMyUIRootLayout* UMyPlayerUISubsystem::GetRootLayout() const
{
	return RootLayout;
}

UMyScreenBase* UMyPlayerUISubsystem::OpenScreen(FName ScreenTag, const FMyUIPayload& Payload)
{
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] OpenScreen Request Tag=%s ContextTag=%s StringValue=%s IntValue=%d"),
		*ScreenTag.ToString(),
		*Payload.ContextTag.ToString(),
		*Payload.StringValue,
		Payload.IntValue);

	if (ScreenTag.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::OpenScreen failed because ScreenTag is None."));
		return nullptr;
	}

	if (!EnsureRootLayout())
	{
		return nullptr;
	}

	UMyUIRegistrySubsystem* RegistrySubsystem = GetRegistrySubsystem();
	if (!RegistrySubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::OpenScreen failed because UIRegistrySubsystem is unavailable."));
		return nullptr;
	}

	FMyUIScreenConfig ScreenConfig;
	if (!RegistrySubsystem->FindScreenConfig(ScreenTag, ScreenConfig))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::OpenScreen failed because ScreenTag '%s' is not registered."), *ScreenTag.ToString());
		return nullptr;
	}

	const FName ResolvedScreenTag = ScreenConfig.ScreenTag.IsNone() ? ScreenTag : ScreenConfig.ScreenTag;
	const EMyUILayer TargetLayer = ScreenConfig.DefaultLayer;

	if (ScreenConfig.bSingletonPerPlayer)
	{
		if (UMyScreenBase* ExistingScreen = FindScreenInstanceByTag(ResolvedScreenTag))
		{
			ExistingScreen->ConfigureScreen(ResolvedScreenTag, TargetLayer);
			ExistingScreen->SetPayload(Payload);

			if (IsCommonStackLayer(TargetLayer))
			{
				if (UCommonActivatableWidgetStack* Stack = GetCommonStackForLayer(TargetLayer))
				{
					const bool bIsAlreadyInStack = Stack->GetWidgetList().Contains(ExistingScreen);
					const bool bIsTopMost = Stack->GetActiveWidget() == ExistingScreen;

					if (bIsAlreadyInStack && !bIsTopMost)
					{
						Stack->RemoveWidget(*ExistingScreen);
					}

					if (!bIsAlreadyInStack || !bIsTopMost)
					{
						Stack->AddWidgetInstance(*ExistingScreen);
					}
					else if (!ExistingScreen->IsActivated())
					{
						ExistingScreen->ActivateWidget();
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::OpenScreen failed because the CommonUI stack for layer '%d' is null."), static_cast<int32>(TargetLayer));
					return nullptr;
				}

				UE_LOG(LogTemp, Display, TEXT("[UIFramework] OpenScreen ReuseSingleton Tag=%s Layer=%s Screen=%s"),
					*ResolvedScreenTag.ToString(),
					LexToString(TargetLayer),
					*GetNameSafe(ExistingScreen));
				DebugLogLayerState(TEXT("OpenScreen.ReuseSingleton.CommonLayer"));
				return ExistingScreen;
			}

			if (RootLayout)
			{
				if (!RootLayout->AddScreenToLayer(ExistingScreen, TargetLayer))
				{
					UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::OpenScreen failed because RootLayout could not re-add singleton '%s' to overlay layer."), *ResolvedScreenTag.ToString());
					return nullptr;
				}
			}

			TArray<TObjectPtr<UMyScreenBase>>& TrackedScreens = LayerScreens.FindOrAdd(TargetLayer);
			TrackedScreens.Remove(ExistingScreen);
			TrackedScreens.Add(ExistingScreen);
			if (!ExistingScreen->IsActivated())
			{
				ExistingScreen->ActivateWidget();
			}
			UE_LOG(LogTemp, Display, TEXT("[UIFramework] OpenScreen ReuseSingleton Tag=%s Layer=%s Screen=%s"),
				*ResolvedScreenTag.ToString(),
				LexToString(TargetLayer),
				*GetNameSafe(ExistingScreen));
			DebugLogLayerState(TEXT("OpenScreen.ReuseSingleton.OverlayLayer"));
			return ExistingScreen;
		}
	}

	TSubclassOf<UMyScreenBase> ScreenClass = ScreenConfig.ScreenClass.LoadSynchronous();
	if (!ScreenClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::OpenScreen failed because ScreenClass for '%s' is null."), *ResolvedScreenTag.ToString());
		return nullptr;
	}

	if (IsCommonStackLayer(TargetLayer))
	{
		UMyScreenBase* NewScreen = PushScreenToCommonLayer(TargetLayer, ScreenClass, Payload, ResolvedScreenTag);
		if (NewScreen && ScreenConfig.bSingletonPerPlayer)
		{
			ScreenInstancesByTag.Add(ResolvedScreenTag, NewScreen);
		}

		DebugLogLayerState(TEXT("OpenScreen.PushCommon"));
		return NewScreen;
	}

	UMyScreenBase* NewScreen = CreateScreenInstance(ScreenClass);
	if (!IsValid(NewScreen))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::OpenScreen failed because CreateWidget returned null for '%s'."), *ResolvedScreenTag.ToString());
		return nullptr;
	}

	NewScreen->ConfigureScreen(ResolvedScreenTag, TargetLayer);
	NewScreen->SetPayload(Payload);

	if (!RootLayout->AddScreenToLayer(NewScreen, TargetLayer))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::OpenScreen failed because RootLayout could not add '%s' to layer."), *ResolvedScreenTag.ToString());
		return nullptr;
	}

	LayerScreens.FindOrAdd(TargetLayer).Add(NewScreen);
	if (ScreenConfig.bSingletonPerPlayer)
	{
		ScreenInstancesByTag.Add(ResolvedScreenTag, NewScreen);
	}

	NewScreen->ActivateWidget();
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] OpenScreen Success Tag=%s Layer=%s Screen=%s"),
		*ResolvedScreenTag.ToString(),
		LexToString(TargetLayer),
		*GetNameSafe(NewScreen));
	DebugLogLayerState(TEXT("OpenScreen.Overlay"));
	return NewScreen;
}

bool UMyPlayerUISubsystem::CloseScreen(UMyScreenBase* Screen)
{
	if (!IsValid(Screen))
	{
		return false;
	}

	const bool bClosingManagedToast = IsManagedToastScreen(Screen);

	if (!Screen->CanBeClosed())
	{
		UE_LOG(LogTemp, Display, TEXT("[UIFramework] CloseScreen Blocked Screen=%s Tag=%s Layer=%s"),
			*GetNameSafe(Screen),
			*Screen->GetScreenTag().ToString(),
			LexToString(Screen->GetPreferredLayer()));
		return false;
	}

	UE_LOG(LogTemp, Display, TEXT("[UIFramework] CloseScreen Start Screen=%s Tag=%s Layer=%s"),
		*GetNameSafe(Screen),
		*Screen->GetScreenTag().ToString(),
		LexToString(Screen->GetPreferredLayer()));

	if (bClosingManagedToast)
	{
		const bool bClosed = CloseManagedToastScreen(Screen);
		RestoreGameplayInputConfigIfNeeded();
		DebugLogLayerState(TEXT("CloseScreen.ManagedToast"));
		return bClosed;
	}

	if (IsCommonStackLayer(Screen->GetPreferredLayer()))
	{
		if (UCommonActivatableWidgetStack* Stack = GetCommonStackForLayer(Screen->GetPreferredLayer()))
		{
			const float RefreshDelay = FMath::Max(0.0f, Stack->GetTransitionDuration());
			Stack->RemoveWidget(*Screen);
			RequestDeferredUIStateRefresh(RefreshDelay);
		}
		else
		{
			Screen->DeactivateWidget();
		}
	}
	else
	{
		if (Screen->IsActivated())
		{
			Screen->DeactivateWidget();
		}

		if (IsValid(RootLayout))
		{
			RootLayout->RemoveScreen(Screen);
		}
		else
		{
			Screen->RemoveFromParent();
		}
	}

	RemoveScreenFromTrackedLayers(Screen);

	RestoreGameplayInputConfigIfNeeded();
	DebugLogLayerState(TEXT("CloseScreen"));
	return true;
}

bool UMyPlayerUISubsystem::CloseTopScreen(EMyUILayer Layer)
{
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] CloseTopScreen Request Layer=%s"), LexToString(Layer));
	if (IsCommonStackLayer(Layer))
	{
		return PopTopScreenFromCommonLayer(Layer);
	}

	if (TArray<TObjectPtr<UMyScreenBase>>* Screens = LayerScreens.Find(Layer))
	{
		for (int32 Index = Screens->Num() - 1; Index >= 0; --Index)
		{
			if (UMyScreenBase* Screen = (*Screens)[Index])
			{
				return CloseScreen(Screen);
			}
		}
	}

	return false;
}

UMyDialogBase* UMyPlayerUISubsystem::ShowConfirmDialog(FMyConfirmDialogConfig Config, FMyDialogResultDelegate Callback)
{
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] ShowConfirmDialog Title=%s Message=%s"),
		*Config.Title.ToString(),
		*Config.Message.ToString());

	if (ConfirmDialogScreenTag.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::ShowConfirmDialog failed because ConfirmDialogScreenTag is None."));
		return nullptr;
	}

	FMyUIPayload Payload;
	UMyScreenBase* Screen = OpenScreen(ConfirmDialogScreenTag, Payload);
	UMyDialogBase* Dialog = Cast<UMyDialogBase>(Screen);
	if (!Dialog)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::ShowConfirmDialog failed because '%s' is not a UMyDialogBase."), *ConfirmDialogScreenTag.ToString());
		return nullptr;
	}

	Dialog->SetupDialog(Config);
	Dialog->SetResultCallback(Callback);
	DebugLogLayerState(TEXT("ShowConfirmDialog"));
	return Dialog;
}

UMyScreenBase* UMyPlayerUISubsystem::ShowDialog(FName ScreenTag, const FMyUIPayload& Payload)
{
	return OpenScreen(ScreenTag, Payload);
}

UMyScreenBase* UMyPlayerUISubsystem::ShowToast(const FText& Message, FGameplayTag ToastTag)
{
	if (DefaultToastScreenTag.IsNone())
	{
		UE_LOG(LogTemp, Display, TEXT("[UIFramework] ToastRequest Tag=%s Message=%s ignored because DefaultToastScreenTag is not configured."),
			*ToastTag.ToString(),
			*Message.ToString());
		return nullptr;
	}

	FMyUIPayload Payload;
	Payload.ContextTag = ToastTag.GetTagName();
	Payload.StringValue = Message.ToString();
	return ShowToastFromPool(Payload);
}

bool UMyPlayerUISubsystem::AddWidgetInstanceToOverlayLayer(UUserWidget* Widget, EMyUILayer Layer)
{
	if (!IsValid(Widget))
	{
		return false;
	}

	if (IsCommonStackLayer(Layer))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::AddWidgetInstanceToOverlayLayer only supports Game/Toast overlay layers."));
		return false;
	}

	if (!EnsureRootLayout())
	{
		return false;
	}

	const bool bAdded = RootLayout->AddWidgetToOverlayLayer(Widget, Layer);
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] AddWidgetInstanceToOverlayLayer Layer=%s Widget=%s Added=%s"),
		LexToString(Layer),
		*GetNameSafe(Widget),
		bAdded ? TEXT("true") : TEXT("false"));
	return bAdded;
}

bool UMyPlayerUISubsystem::RemoveOverlayWidget(UUserWidget* Widget)
{
	if (!IsValid(Widget))
	{
		return false;
	}

	if (RootLayout)
	{
		const bool bRemoved = RootLayout->RemoveWidgetFromLayout(Widget);
		UE_LOG(LogTemp, Display, TEXT("[UIFramework] RemoveOverlayWidget Widget=%s Removed=%s"),
			*GetNameSafe(Widget),
			bRemoved ? TEXT("true") : TEXT("false"));
		return bRemoved;
	}

	Widget->RemoveFromParent();
	return true;
}

void UMyPlayerUISubsystem::SetRootLayoutClass(TSoftClassPtr<UMyUIRootLayout> InRootLayoutClass)
{
	RootLayoutClass = InRootLayoutClass;
}

void UMyPlayerUISubsystem::SetConfirmDialogScreenTag(FName InScreenTag)
{
	ConfirmDialogScreenTag = InScreenTag;
}

UMyScreenBase* UMyPlayerUISubsystem::FindScreenByTag(FName ScreenTag) const
{
	return FindScreenInstanceByTag(ScreenTag);
}

UMyUIRegistrySubsystem* UMyPlayerUISubsystem::GetRegistrySubsystem() const
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = LocalPlayer->GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UMyUIRegistrySubsystem>() : nullptr;
}

UHoyoObjectPoolSubsystem* UMyPlayerUISubsystem::GetObjectPoolSubsystem() const
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = LocalPlayer->GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UHoyoObjectPoolSubsystem>() : nullptr;
}

TSubclassOf<UMyUIRootLayout> UMyPlayerUISubsystem::ResolveRootLayoutClass() const
{
	if (RootLayoutClass.IsNull())
	{
		return UMyUIRootLayout::StaticClass();
	}

	return RootLayoutClass.LoadSynchronous();
}

UCommonActivatableWidgetStack* UMyPlayerUISubsystem::GetCommonStackForLayer(EMyUILayer Layer) const
{
	if (!RootLayout)
	{
		return nullptr;
	}

	switch (Layer)
	{
	case EMyUILayer::Menu:
		return RootLayout->GetMenuStack();
	case EMyUILayer::Modal:
		return RootLayout->GetModalStack();
	default:
		return nullptr;
	}
}

bool UMyPlayerUISubsystem::IsCommonStackLayer(EMyUILayer Layer) const
{
	return Layer == EMyUILayer::Menu || Layer == EMyUILayer::Modal;
}

UMyScreenBase* UMyPlayerUISubsystem::CreateScreenInstance(TSubclassOf<UMyScreenBase> ScreenClass) const
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = LocalPlayer->GetGameInstance();
	APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld());

	if (PlayerController)
	{
		return CreateWidget<UMyScreenBase>(PlayerController, ScreenClass);
	}

	if (GameInstance)
	{
		return CreateWidget<UMyScreenBase>(GameInstance, ScreenClass);
	}

	return nullptr;
}

UMyScreenBase* UMyPlayerUISubsystem::FindScreenInstanceByTag(FName ScreenTag) const
{
	if (const TObjectPtr<UMyScreenBase>* FoundScreen = ScreenInstancesByTag.Find(ScreenTag))
	{
		return FoundScreen->Get();
	}

	for (const TPair<EMyUILayer, TArray<TObjectPtr<UMyScreenBase>>>& Pair : LayerScreens)
	{
		for (UMyScreenBase* Screen : Pair.Value)
		{
			if (IsValid(Screen) && Screen->GetScreenTag() == ScreenTag)
			{
				return Screen;
			}
		}
	}

	return nullptr;
}

void UMyPlayerUISubsystem::RestoreGameplayInputConfigIfNeeded() const
{
	const UCommonActivatableWidgetStack* MenuLayerStack = GetCommonStackForLayer(EMyUILayer::Menu);
	const UCommonActivatableWidgetStack* ModalLayerStack = GetCommonStackForLayer(EMyUILayer::Modal);

	const int32 MenuStackCount = MenuLayerStack ? MenuLayerStack->GetNumWidgets() : 0;
	const int32 ModalStackCount = ModalLayerStack ? ModalLayerStack->GetNumWidgets() : 0;
	const bool bHasMenuWidgets = MenuStackCount > 0;
	const bool bHasModalWidgets = ModalStackCount > 0;
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] RestoreGameplayInputConfigIfNeeded MenuStackCount=%d MenuTop=%s ModalStackCount=%d ModalTop=%s"),
		MenuStackCount,
		*GetNameSafe(MenuLayerStack ? MenuLayerStack->GetActiveWidget() : nullptr),
		ModalStackCount,
		*GetNameSafe(ModalLayerStack ? ModalLayerStack->GetActiveWidget() : nullptr));

	if (bHasMenuWidgets || bHasModalWidgets)
	{
		return;
	}

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UCommonUIActionRouterBase* ActionRouter = LocalPlayer->GetSubsystem<UCommonUIActionRouterBase>())
		{
			FUIInputConfig GameplayConfig(ECommonInputMode::Game, EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown, EMouseLockMode::LockAlways, true);
			GameplayConfig.bIgnoreMoveInput = false;
			GameplayConfig.bIgnoreLookInput = false;
			ActionRouter->SetActiveUIInputConfig(GameplayConfig, this);
			UE_LOG(LogTemp, Display, TEXT("[UIFramework] RestoreGameplayInputConfig Applied Mode=Game IgnoreMove=false IgnoreLook=false"));
		}

		if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
		{
			FInputModeGameOnly InputMode;
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(false);
			PlayerController->SetIgnoreMoveInput(false);
			PlayerController->SetIgnoreLookInput(false);
			UE_LOG(LogTemp, Display, TEXT("[UIFramework] RestoreGameplayInputConfig Applied PlayerController fallback GameOnly Cursor=false"));
		}
	}
}

void UMyPlayerUISubsystem::RequestDeferredUIStateRefresh(float DelaySeconds) const
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		FTimerDelegate DeferredRefreshDelegate = FTimerDelegate::CreateWeakLambda(this,
			[this]()
			{
				RestoreGameplayInputConfigIfNeeded();
				DebugLogLayerState(TEXT("DeferredUIStateRefresh"));
			});

		if (DelaySeconds <= 0.0f)
		{
			TimerManager.SetTimerForNextTick(DeferredRefreshDelegate);
		}
		else
		{
			FTimerHandle TimerHandle;
			TimerManager.SetTimer(TimerHandle, DeferredRefreshDelegate, DelaySeconds, false);
		}

		UE_LOG(LogTemp, Display, TEXT("[UIFramework] RequestDeferredUIStateRefresh Delay=%.3f"), DelaySeconds);
	}
}

void UMyPlayerUISubsystem::BindCommonStackDebugHooks()
{
	if (!RootLayout)
	{
		return;
	}

	if (UCommonActivatableWidgetStack* MenuLayerStack = RootLayout->GetMenuStack())
	{
		MenuLayerStack->OnDisplayedWidgetChanged().RemoveAll(this);
		MenuLayerStack->OnDisplayedWidgetChanged().AddUObject(this, &UMyPlayerUISubsystem::HandleMenuStackDisplayedWidgetChanged);
	}

	if (UCommonActivatableWidgetStack* ModalLayerStack = RootLayout->GetModalStack())
	{
		ModalLayerStack->OnDisplayedWidgetChanged().RemoveAll(this);
		ModalLayerStack->OnDisplayedWidgetChanged().AddUObject(this, &UMyPlayerUISubsystem::HandleModalStackDisplayedWidgetChanged);
	}
}

void UMyPlayerUISubsystem::HandleMenuStackDisplayedWidgetChanged(UCommonActivatableWidget* DisplayedWidget)
{
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] DisplayedWidgetChanged Layer=Menu Widget=%s"), *GetNameSafe(DisplayedWidget));
	if (DisplayedWidget)
	{
		ApplyFocusToDisplayedWidget(DisplayedWidget, EMyUILayer::Menu);
	}
	else
	{
		RestoreGameplayInputConfigIfNeeded();
		RequestDeferredUIStateRefresh(0.0f);
	}
	DebugLogLayerState(TEXT("DisplayedWidgetChanged.Menu"));
}

void UMyPlayerUISubsystem::HandleModalStackDisplayedWidgetChanged(UCommonActivatableWidget* DisplayedWidget)
{
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] DisplayedWidgetChanged Layer=Modal Widget=%s"), *GetNameSafe(DisplayedWidget));
	if (DisplayedWidget)
	{
		ApplyFocusToDisplayedWidget(DisplayedWidget, EMyUILayer::Modal);
	}
	else
	{
		RestoreGameplayInputConfigIfNeeded();
		RequestDeferredUIStateRefresh(0.0f);
	}
	DebugLogLayerState(TEXT("DisplayedWidgetChanged.Modal"));
}

void UMyPlayerUISubsystem::ApplyFocusToDisplayedWidget(UCommonActivatableWidget* DisplayedWidget, EMyUILayer Layer)
{
	if (!DisplayedWidget)
	{
		return;
	}

	UWidget* DesiredFocusTarget = DisplayedWidget->GetDesiredFocusTarget();
	UWidget* FocusWidget = DesiredFocusTarget ? DesiredFocusTarget : Cast<UWidget>(DisplayedWidget);
	ApplyFocusToWidget(FocusWidget, DisplayedWidget, Layer, TEXT("Immediate"));

	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		FTimerDelegate DeferredFocusDelegate = FTimerDelegate::CreateWeakLambda(this,
			[this, FocusWidget, DisplayedWidget, Layer]()
			{
				ApplyFocusToWidget(FocusWidget, DisplayedWidget, Layer, TEXT("Deferred"));
			});
		TimerManager.SetTimerForNextTick(DeferredFocusDelegate);
	}
}

void UMyPlayerUISubsystem::ApplyFocusToWidget(UWidget* FocusWidget, UCommonActivatableWidget* OwnerWidget, EMyUILayer Layer, const TCHAR* Phase)
{
	if (!FocusWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIFramework] ApplyFocusToWidget Phase=%s Layer=%s Owner=%s failed because FocusWidget is null."),
			Phase,
			LexToString(Layer),
			*GetNameSafe(OwnerWidget));
		return;
	}

	APlayerController* PlayerController = GetLocalPlayer() ? GetLocalPlayer()->GetPlayerController(GetWorld()) : nullptr;
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIFramework] ApplyFocusToWidget Phase=%s Layer=%s Owner=%s failed because PlayerController is null."),
			Phase,
			LexToString(Layer),
			*GetNameSafe(OwnerWidget));
		return;
	}

	FocusWidget->SetUserFocus(PlayerController);
	FocusWidget->SetKeyboardFocus();
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] ApplyFocusToWidget Phase=%s Layer=%s Owner=%s FocusTarget=%s"),
		Phase,
		LexToString(Layer),
		*GetNameSafe(OwnerWidget),
		*GetNameSafe(FocusWidget));
}

UMyScreenBase* UMyPlayerUISubsystem::PushScreenToCommonLayer(EMyUILayer Layer, TSubclassOf<UMyScreenBase> ScreenClass, const FMyUIPayload& Payload, FName ScreenTag)
{
	UCommonActivatableWidgetStack* Stack = GetCommonStackForLayer(Layer);
	if (!Stack)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::PushScreenToCommonLayer failed because target stack is null."));
		return nullptr;
	}

	UMyScreenBase* NewScreen = CreateScreenInstance(ScreenClass);
	if (!IsValid(NewScreen))
	{
		return nullptr;
	}

	NewScreen->ConfigureScreen(ScreenTag, Layer);
	NewScreen->SetPayload(Payload);
	Stack->AddWidgetInstance(*NewScreen);
	LayerScreens.FindOrAdd(Layer).Add(NewScreen);
	UE_LOG(LogTemp, Display, TEXT("[UIFramework] PushScreenToCommonLayer Layer=%s Screen=%s Tag=%s NewTop=%s"),
		LexToString(Layer),
		*GetNameSafe(NewScreen),
		*ScreenTag.ToString(),
		*GetNameSafe(Stack->GetActiveWidget()));
	return NewScreen;
}

bool UMyPlayerUISubsystem::PopTopScreenFromCommonLayer(EMyUILayer Layer)
{
	if (UCommonActivatableWidgetStack* Stack = GetCommonStackForLayer(Layer))
	{
		UE_LOG(LogTemp, Display, TEXT("[UIFramework] PopTopScreenFromCommonLayer Layer=%s CurrentTop=%s"),
			LexToString(Layer),
			*GetNameSafe(Stack->GetActiveWidget()));
		if (UMyScreenBase* TopScreen = Cast<UMyScreenBase>(Stack->GetActiveWidget()))
		{
			return CloseScreen(TopScreen);
		}
	}

	return false;
}

void UMyPlayerUISubsystem::RemoveScreenFromTrackedLayers(UMyScreenBase* Screen)
{
	for (TPair<EMyUILayer, TArray<TObjectPtr<UMyScreenBase>>>& Pair : LayerScreens)
	{
		Pair.Value.Remove(Screen);
	}
}

bool UMyPlayerUISubsystem::IsManagedToastScreen(const UMyScreenBase* Screen) const
{
	if (!IsValid(Screen) || Screen->GetPreferredLayer() != EMyUILayer::Toast || Screen->GetScreenTag() != DefaultToastScreenTag)
	{
		return false;
	}

	for (const UMyScreenBase* ActiveToastScreen : ActiveToastScreens)
	{
		if (ActiveToastScreen == Screen)
		{
			return true;
		}
	}

	return false;
}

UMyScreenBase* UMyPlayerUISubsystem::ShowToastFromPool(const FMyUIPayload& Payload)
{
	if (DefaultToastScreenTag.IsNone())
	{
		return nullptr;
	}

	if (!EnsureRootLayout())
	{
		return nullptr;
	}

	UMyUIRegistrySubsystem* RegistrySubsystem = GetRegistrySubsystem();
	if (!RegistrySubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::ShowToastFromPool failed because UIRegistrySubsystem is unavailable."));
		return nullptr;
	}

	FMyUIScreenConfig ScreenConfig;
	if (!RegistrySubsystem->FindScreenConfig(DefaultToastScreenTag, ScreenConfig))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::ShowToastFromPool failed because DefaultToastScreenTag '%s' is not registered."), *DefaultToastScreenTag.ToString());
		return nullptr;
	}

	const FName ResolvedScreenTag = ScreenConfig.ScreenTag.IsNone() ? DefaultToastScreenTag : ScreenConfig.ScreenTag;
	TSubclassOf<UMyScreenBase> ScreenClass = ScreenConfig.ScreenClass.LoadSynchronous();
	if (!ScreenClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::ShowToastFromPool failed because ScreenClass for '%s' is null."), *ResolvedScreenTag.ToString());
		return nullptr;
	}

	UHoyoObjectPoolSubsystem* ObjectPoolSubsystem = GetObjectPoolSubsystem();
	if (!ObjectPoolSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::ShowToastFromPool failed because ObjectPoolSubsystem is unavailable."));
		return nullptr;
	}

	const int32 MaxVisibleCount = FMath::Max(1, MaxVisibleToastCount);
	while (ActiveToastScreens.Num() >= MaxVisibleCount)
	{
		CloseManagedToastScreen(ActiveToastScreens[0]);
	}

	UUserWidget* PooledWidget = ObjectPoolSubsystem->AcquireUserWidget(TSubclassOf<UUserWidget>(ScreenClass.Get()), GetLocalPlayer());
	UMyScreenBase* ToastScreen = Cast<UMyScreenBase>(PooledWidget);
	if (!IsValid(ToastScreen))
	{
		if (IsValid(PooledWidget))
		{
			ObjectPoolSubsystem->ReleaseUserWidget(PooledWidget);
		}

		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::ShowToastFromPool failed because pooled widget is not a UMyScreenBase."));
		return nullptr;
	}

	FMyUIPayload StackPayload = Payload;
	StackPayload.IntValue = ActiveToastScreens.Num();

	ToastScreen->ConfigureScreen(ResolvedScreenTag, EMyUILayer::Toast);
	ToastScreen->SetPayload(StackPayload);

	if (!RootLayout->AddScreenToLayer(ToastScreen, EMyUILayer::Toast))
	{
		ObjectPoolSubsystem->ReleaseUserWidget(ToastScreen);
		UE_LOG(LogTemp, Warning, TEXT("UMyPlayerUISubsystem::ShowToastFromPool failed because RootLayout could not add toast to layer."));
		return nullptr;
	}

	LayerScreens.FindOrAdd(EMyUILayer::Toast).Add(ToastScreen);
	ActiveToastScreens.Add(ToastScreen);
	ToastScreen->ActivateWidget();
	ReflowToastStack();

	UE_LOG(LogTemp, Display, TEXT("[UIFramework] ToastDisplayed Screen=%s StackCount=%d Message=%s"),
		*GetNameSafe(ToastScreen),
		ActiveToastScreens.Num(),
		*Payload.StringValue);
	return ToastScreen;
}

bool UMyPlayerUISubsystem::CloseManagedToastScreen(UMyScreenBase* Screen)
{
	if (!IsValid(Screen))
	{
		return false;
	}

	if (Screen->IsActivated())
	{
		Screen->DeactivateWidget();
	}

	if (IsValid(RootLayout))
	{
		RootLayout->RemoveScreen(Screen);
	}
	else
	{
		Screen->RemoveFromParent();
	}

	RemoveScreenFromTrackedLayers(Screen);
	ActiveToastScreens.Remove(Screen);

	if (UHoyoObjectPoolSubsystem* ObjectPoolSubsystem = GetObjectPoolSubsystem())
	{
		ObjectPoolSubsystem->ReleaseUserWidget(Screen);
		if (MaxIdleToastPoolCount >= 0)
		{
			ObjectPoolSubsystem->TrimUserWidgetPool(Screen->GetClass(), MaxIdleToastPoolCount);
		}
	}

	ReflowToastStack();
	return true;
}

void UMyPlayerUISubsystem::ReflowToastStack()
{
	for (int32 Index = 0; Index < ActiveToastScreens.Num(); ++Index)
	{
		UMyScreenBase* ToastScreen = ActiveToastScreens[Index];
		if (!IsValid(ToastScreen))
		{
			continue;
		}

		if (UMyToastScreenBase* TypedToastScreen = Cast<UMyToastScreenBase>(ToastScreen))
		{
			TypedToastScreen->SetToastStackIndex(Index);
		}
	}
}

void UMyPlayerUISubsystem::DebugLogLayerState(const TCHAR* Context) const
{
	const UCommonActivatableWidgetStack* MenuLayerStack = GetCommonStackForLayer(EMyUILayer::Menu);
	const UCommonActivatableWidgetStack* ModalLayerStack = GetCommonStackForLayer(EMyUILayer::Modal);
	const TArray<TObjectPtr<UMyScreenBase>>* GameScreens = LayerScreens.Find(EMyUILayer::Game);
	const TArray<TObjectPtr<UMyScreenBase>>* ToastScreens = LayerScreens.Find(EMyUILayer::Toast);
	const int32 MenuStackCount = MenuLayerStack ? MenuLayerStack->GetNumWidgets() : 0;
	const int32 ModalStackCount = ModalLayerStack ? ModalLayerStack->GetNumWidgets() : 0;

	const int32 GameCount = GameScreens ? GameScreens->Num() : 0;
	const int32 MenuCount = LayerScreens.Contains(EMyUILayer::Menu) ? LayerScreens.FindChecked(EMyUILayer::Menu).Num() : 0;
	const int32 ModalCount = LayerScreens.Contains(EMyUILayer::Modal) ? LayerScreens.FindChecked(EMyUILayer::Modal).Num() : 0;
	const int32 ToastCount = ToastScreens ? ToastScreens->Num() : 0;
	const UMyScreenBase* GameTop = (GameScreens && GameScreens->Num() > 0) ? GameScreens->Last().Get() : nullptr;
	const UMyScreenBase* ToastTop = (ToastScreens && ToastScreens->Num() > 0) ? ToastScreens->Last().Get() : nullptr;

	UE_LOG(LogTemp, Display, TEXT("[UIFramework] StackState Context=%s GameCount=%d GameTop=%s MenuCount=%d MenuStackCount=%d MenuTop=%s ModalCount=%d ModalStackCount=%d ModalTop=%s ToastCount=%d ToastTop=%s"),
		Context,
		GameCount,
		*GetNameSafe(GameTop),
		MenuCount,
		MenuStackCount,
		*GetNameSafe(MenuLayerStack ? MenuLayerStack->GetActiveWidget() : nullptr),
		ModalCount,
		ModalStackCount,
		*GetNameSafe(ModalLayerStack ? ModalLayerStack->GetActiveWidget() : nullptr),
		ToastCount,
		*GetNameSafe(ToastTop));
}
