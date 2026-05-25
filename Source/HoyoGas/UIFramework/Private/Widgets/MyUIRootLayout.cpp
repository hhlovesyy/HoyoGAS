#include "Widgets/MyUIRootLayout.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/PanelWidget.h"
#include "Components/Widget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Widgets/MyScreenBase.h"

void UMyUIRootLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureLayerRoots();
}

bool UMyUIRootLayout::AddScreenToLayer(UMyScreenBase* Screen, EMyUILayer Layer)
{
	return AddWidgetToOverlayLayerInternal(Screen, Layer, true);
}

bool UMyUIRootLayout::RemoveScreen(UMyScreenBase* Screen)
{
	return RemoveWidgetFromLayout(Screen);
}

bool UMyUIRootLayout::AddWidgetToOverlayLayer(UWidget* Widget, EMyUILayer Layer)
{
	return AddWidgetToOverlayLayerInternal(Widget, Layer, false);
}

bool UMyUIRootLayout::RemoveWidgetFromLayout(UWidget* Widget)
{
	if (!IsValid(Widget))
	{
		return false;
	}

	if (UPanelWidget* ParentPanel = Cast<UPanelWidget>(Widget->GetParent()))
	{
		return ParentPanel->RemoveChild(Widget);
	}

	Widget->RemoveFromParent();
	return true;
}

UOverlay* UMyUIRootLayout::GetLayerRoot(EMyUILayer Layer) const
{
	switch (Layer)
	{
	case EMyUILayer::Game:
		return GameLayerRoot;
	case EMyUILayer::Toast:
		return ToastLayerRoot;
	default:
		return nullptr;
	}
}

UCommonActivatableWidgetStack* UMyUIRootLayout::GetMenuStack() const
{
	return MenuStack;
}

UCommonActivatableWidgetStack* UMyUIRootLayout::GetModalStack() const
{
	return ModalStack;
}

bool UMyUIRootLayout::AddWidgetToOverlayLayerInternal(UWidget* Widget, EMyUILayer Layer, bool bFillWidget)
{
	if (!IsValid(Widget))
	{
		return false;
	}

	UOverlay* LayerRoot = GetLayerRoot(Layer);
	if (!LayerRoot)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyUIRootLayout::AddWidgetToOverlayLayerInternal only supports Game/Toast overlay layers."));
		return false;
	}

	Widget->RemoveFromParent();

	if (UOverlaySlot* OverlaySlot = LayerRoot->AddChildToOverlay(Widget))
	{
		OverlaySlot->SetHorizontalAlignment(bFillWidget ? HAlign_Fill : HAlign_Left);
		OverlaySlot->SetVerticalAlignment(bFillWidget ? VAlign_Fill : VAlign_Top);
		return true;
	}

	return false;
}

void UMyUIRootLayout::EnsureLayerRoots()
{
	if (GameLayerRoot && MenuStack && ModalStack && ToastLayerRoot)
	{
		return;
	}

	if (!WidgetTree)
	{
		return;
	}

	UOverlay* RootOverlay = Cast<UOverlay>(WidgetTree->RootWidget);
	if (!RootOverlay)
	{
		RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
		WidgetTree->RootWidget = RootOverlay;
	}

	if (!GameLayerRoot)
	{
		GameLayerRoot = CreateLayerRoot(TEXT("GameLayerRoot"));
		if (UOverlaySlot* OverlaySlot = RootOverlay->AddChildToOverlay(GameLayerRoot))
		{
			OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			OverlaySlot->SetVerticalAlignment(VAlign_Fill);
		}
	}

	if (!MenuStack)
	{
		MenuStack = CreateCommonStack(TEXT("MenuStack"));
		if (UOverlaySlot* OverlaySlot = RootOverlay->AddChildToOverlay(MenuStack))
		{
			OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			OverlaySlot->SetVerticalAlignment(VAlign_Fill);
		}
	}

	if (!ModalStack)
	{
		ModalStack = CreateCommonStack(TEXT("ModalStack"));
		if (UOverlaySlot* OverlaySlot = RootOverlay->AddChildToOverlay(ModalStack))
		{
			OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			OverlaySlot->SetVerticalAlignment(VAlign_Fill);
		}
	}

	if (!ToastLayerRoot)
	{
		ToastLayerRoot = CreateLayerRoot(TEXT("ToastLayerRoot"));
		if (UOverlaySlot* OverlaySlot = RootOverlay->AddChildToOverlay(ToastLayerRoot))
		{
			OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			OverlaySlot->SetVerticalAlignment(VAlign_Fill);
		}
	}
}

UOverlay* UMyUIRootLayout::CreateLayerRoot(const FName InName)
{
	UOverlay* NewLayerRoot = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), InName);
	NewLayerRoot->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	return NewLayerRoot;
}

UCommonActivatableWidgetStack* UMyUIRootLayout::CreateCommonStack(const FName InName)
{
	UCommonActivatableWidgetStack* NewStack = WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>(UCommonActivatableWidgetStack::StaticClass(), InName);
	NewStack->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	return NewStack;
}
