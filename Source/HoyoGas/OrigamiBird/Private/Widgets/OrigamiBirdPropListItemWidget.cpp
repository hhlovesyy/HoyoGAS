#include "Widgets/OrigamiBirdPropListItemWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "INotifyFieldValueChanged.h"
#include "MVVMSubsystem.h"
#include "View/MVVMView.h"
#include "ViewModels/VM_OrigamiBirdPropEntry.h"

namespace
{
	void SetPropEntryWidgetViewModel(UUserWidget* Widget, UObject* InViewModel)
	{
		if (!Widget || !InViewModel)
		{
			return;
		}

		if (UMVVMView* View = UMVVMSubsystem::GetViewFromUserWidget(Widget))
		{
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

void UOrigamiBirdPropListItemWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureDefaultVisualTree();
	RefreshVisuals();
}

void UOrigamiBirdPropListItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	BindEntryViewModel(Cast<UVM_OrigamiBirdPropEntry>(ListItemObject));
}

void UOrigamiBirdPropListItemWidget::NativeOnItemSelectionChanged(bool bIsSelected)
{
	IUserListEntry::NativeOnItemSelectionChanged(bIsSelected);

	if (RowBorder)
	{
		RowBorder->SetBrushColor(bIsSelected
			? FLinearColor(0.18f, 0.28f, 0.42f, 0.96f)
			: FLinearColor(0.055f, 0.065f, 0.075f, 0.92f));
	}
}

void UOrigamiBirdPropListItemWidget::NativeOnEntryReleased()
{
	IUserListEntry::NativeOnEntryReleased();
	BindEntryViewModel(nullptr);
}

void UOrigamiBirdPropListItemWidget::EnsureDefaultVisualTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	RowBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RowBorder"));
	UHorizontalBox* RootBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("RootBox"));
	USizeBox* IconSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("IconSizeBox"));
	IconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("IconImage"));
	UVerticalBox* TextStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TextStack"));
	UHorizontalBox* NameLine = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("NameLine"));
	DisplayNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DisplayNameText"));
	CountText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CountText"));
	DescriptionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DescriptionText"));
	StackRuleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StackRuleText"));

	WidgetTree->RootWidget = RowBorder;
	RowBorder->SetPadding(FMargin(10.0f, 8.0f));
	RowBorder->SetBrushColor(FLinearColor(0.055f, 0.065f, 0.075f, 0.92f));
	RowBorder->SetContent(RootBox);

	IconSizeBox->SetWidthOverride(52.0f);
	IconSizeBox->SetHeightOverride(52.0f);
	IconSizeBox->SetContent(IconImage);

	if (UHorizontalBoxSlot* IconSlot = RootBox->AddChildToHorizontalBox(IconSizeBox))
	{
		IconSlot->SetPadding(FMargin(0.0f, 0.0f, 10.0f, 0.0f));
		IconSlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UHorizontalBoxSlot* TextSlot = RootBox->AddChildToHorizontalBox(TextStack))
	{
		TextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		TextSlot->SetVerticalAlignment(VAlign_Center);
	}

	if (UHorizontalBoxSlot* NameSlot = NameLine->AddChildToHorizontalBox(DisplayNameText))
	{
		NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	if (UHorizontalBoxSlot* CountSlot = NameLine->AddChildToHorizontalBox(CountText))
	{
		CountSlot->SetPadding(FMargin(8.0f, 0.0f, 0.0f, 0.0f));
	}

	TextStack->AddChildToVerticalBox(NameLine);
	TextStack->AddChildToVerticalBox(DescriptionText);
	TextStack->AddChildToVerticalBox(StackRuleText);

	DisplayNameText->SetColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.93f, 0.84f, 1.0f)));
	CountText->SetColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.80f, 0.42f, 1.0f)));
	DescriptionText->SetColorAndOpacity(FSlateColor(FLinearColor(0.72f, 0.76f, 0.78f, 1.0f)));
	StackRuleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.52f, 0.64f, 0.78f, 1.0f)));
}

void UOrigamiBirdPropListItemWidget::BindEntryViewModel(UVM_OrigamiBirdPropEntry* InEntryViewModel)
{
	CachedPropEntryViewModel = InEntryViewModel;
	SetPropEntryWidgetViewModel(this, CachedPropEntryViewModel);
	RefreshVisuals();
}

void UOrigamiBirdPropListItemWidget::RefreshVisuals()
{
	if (DisplayNameText)
	{
		DisplayNameText->SetText(CachedPropEntryViewModel ? CachedPropEntryViewModel->GetDisplayNameText() : FText::GetEmpty());
	}
	if (CountText)
	{
		CountText->SetText(CachedPropEntryViewModel ? CachedPropEntryViewModel->GetCountText() : FText::GetEmpty());
	}
	if (DescriptionText)
	{
		DescriptionText->SetText(CachedPropEntryViewModel ? CachedPropEntryViewModel->GetDescriptionText() : FText::GetEmpty());
	}
	if (StackRuleText)
	{
		StackRuleText->SetText(CachedPropEntryViewModel ? CachedPropEntryViewModel->GetStackRuleText() : FText::GetEmpty());
	}
	if (IconImage)
	{
		if (CachedPropEntryViewModel && CachedPropEntryViewModel->GetIconTexture())
		{
			IconImage->SetBrushFromTexture(CachedPropEntryViewModel->GetIconTexture(), true);
			IconImage->SetColorAndOpacity(FLinearColor::White);
		}
		else
		{
			IconImage->SetColorAndOpacity(FLinearColor(0.28f, 0.34f, 0.40f, 1.0f));
		}
	}
}
