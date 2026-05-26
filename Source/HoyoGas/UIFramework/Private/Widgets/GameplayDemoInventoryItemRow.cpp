#include "Widgets/GameplayDemoInventoryItemRow.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "INotifyFieldValueChanged.h"
#include "MVVMSubsystem.h"
#include "View/MVVMView.h"
#include "VM_InventoryItemEntry.h"

namespace
{
	void SetInventoryItemWidgetViewModel(UUserWidget* Widget, UObject* InViewModel)
	{
		if (!Widget)
		{
			return;
		}

		if (UMVVMView* View = UMVVMSubsystem::GetViewFromUserWidget(Widget))
		{
			if (!InViewModel)
			{
				// Entry widgets are recycled by the list view. Leave the previous source in place
				// until the next item assignment or widget destruction so UMVVMView can tear it down once.
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

void UGameplayDemoInventoryItemRow::ApplyItemTexture(UTexture2D* InTexture)
{
	if (ItemImage)
	{
		ItemImage->SetBrushFromTexture(InTexture, true);
	}
}

void UGameplayDemoInventoryItemRow::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	ViewModel = Cast<UVM_InventoryItemEntry>(ListItemObject);
	SetInventoryItemWidgetViewModel(this, ViewModel);
}

void UGameplayDemoInventoryItemRow::NativeOnItemSelectionChanged(bool bIsSelected)
{
	IUserListEntry::NativeOnItemSelectionChanged(bIsSelected);

	if (RowBorder)
	{
		RowBorder->SetBrushColor(bIsSelected ? FLinearColor(0.18f, 0.32f, 0.58f, 1.0f) : FLinearColor(0.05f, 0.07f, 0.11f, 0.92f));
	}
}

void UGameplayDemoInventoryItemRow::NativeOnEntryReleased()
{
	IUserListEntry::NativeOnEntryReleased();
	ViewModel = nullptr;
	SetInventoryItemWidgetViewModel(this, nullptr);
}
