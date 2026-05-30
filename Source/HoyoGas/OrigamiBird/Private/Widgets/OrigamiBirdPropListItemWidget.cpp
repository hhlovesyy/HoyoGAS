#include "Widgets/OrigamiBirdPropListItemWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "INotifyFieldValueChanged.h"
#include "MVVMSubsystem.h"
#include "View/MVVMView.h"
#include "ViewModels/VM_OrigamiBirdPropEntry.h"

namespace
{
	void SetPropEntryWidgetViewModel(UUserWidget* Widget, UObject* InViewModel)
	{
		if (!Widget)
		{
			return;
		}

		if (UMVVMView* View = UMVVMSubsystem::GetViewFromUserWidget(Widget))
		{
			if (!InViewModel)
			{
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

void UOrigamiBirdPropListItemWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	bCachedIsSelected = IsListItemSelected();
	ApplySelectionVisual();
}

void UOrigamiBirdPropListItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	BindEntryViewModel(Cast<UVM_OrigamiBirdPropEntry>(ListItemObject));
}

void UOrigamiBirdPropListItemWidget::NativeOnItemSelectionChanged(bool bIsSelected)
{
	IUserListEntry::NativeOnItemSelectionChanged(bIsSelected);

	bCachedIsSelected = bIsSelected;
	ApplySelectionVisual();
}

void UOrigamiBirdPropListItemWidget::NativeOnEntryReleased()
{
	IUserListEntry::NativeOnEntryReleased();

	bCachedIsSelected = false;
	BindEntryViewModel(nullptr);
}

void UOrigamiBirdPropListItemWidget::BindEntryViewModel(UVM_OrigamiBirdPropEntry* InEntryViewModel)
{
	CachedPropEntryViewModel = InEntryViewModel;
	bCachedIsSelected = IsListItemSelected();
	SetPropEntryWidgetViewModel(this, CachedPropEntryViewModel);
	ApplyIconVisual();
	ApplySelectionVisual();
}

void UOrigamiBirdPropListItemWidget::ApplyIconVisual()
{
	if (!IconImage)
	{
		return;
	}

	if (CachedPropEntryViewModel && CachedPropEntryViewModel->GetIconTexture())
	{
		IconImage->SetBrushFromTexture(CachedPropEntryViewModel->GetIconTexture(), true);
		IconImage->SetColorAndOpacity(FLinearColor::White);
		return;
	}

	IconImage->SetColorAndOpacity(FLinearColor(0.28f, 0.34f, 0.40f, 1.0f));
}

void UOrigamiBirdPropListItemWidget::ApplySelectionVisual()
{
	if (!RowBorder)
	{
		return;
	}

	RowBorder->SetBrushColor(
		bCachedIsSelected
			? FLinearColor(0.18f, 0.28f, 0.42f, 0.96f)
			: FLinearColor(0.055f, 0.065f, 0.075f, 0.92f));
}
