#include "VM_InventoryScreen.h"

#include "Stores/InventoryUIStore.h"
#include "VM_InventoryItemEntry.h"

void UVM_InventoryScreen::Initialize(UInventoryUIStore* InInventoryStore)
{
	if (InventoryStore.Get() != InInventoryStore)
	{
		if (InventoryStore.IsValid())
		{
			InventoryStore->OnStoreChanged().RemoveAll(this);
		}

		InventoryStore = InInventoryStore;
		if (InventoryStore.IsValid())
		{
			InventoryStore->OnStoreChanged().AddUObject(this, &UVM_InventoryScreen::HandleStoreChanged);
		}
	}

	RefreshFromStore();
}

void UVM_InventoryScreen::Teardown()
{
	if (InventoryStore.IsValid())
	{
		InventoryStore->OnStoreChanged().RemoveAll(this);
		InventoryStore.Reset();
	}
}

const TArray<TObjectPtr<UVM_InventoryItemEntry>>& UVM_InventoryScreen::GetItemEntries() const
{
	return ItemEntries;
}

void UVM_InventoryScreen::SetItemEntries(const TArray<TObjectPtr<UVM_InventoryItemEntry>>& InEntries)
{
	ItemEntries = InEntries;
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ItemEntries);

	if (ItemEntries.IsEmpty())
	{
		SetSelectedIndex(INDEX_NONE);
		return;
	}

	if (!ItemEntries.IsValidIndex(SelectedIndex))
	{
		SetSelectedIndex(0);
	}
	else
	{
		RefreshSelectionDetails();
	}
}

FText UVM_InventoryScreen::GetTotalItemCountText() const
{
	return TotalItemCountText;
}

void UVM_InventoryScreen::SetTotalItemCountText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(TotalItemCountText, InValue);
}

FText UVM_InventoryScreen::GetTotalValueText() const
{
	return TotalValueText;
}

void UVM_InventoryScreen::SetTotalValueText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(TotalValueText, InValue);
}

int32 UVM_InventoryScreen::GetSelectedIndex() const
{
	return SelectedIndex;
}

void UVM_InventoryScreen::SetSelectedIndex(int32 InValue)
{
	if (InventoryStore.IsValid() && InventoryStore->GetSelectedIndex() != InValue)
	{
		InventoryStore->SetSelectedIndex(InValue);
	}
	else
	{
		UE_MVVM_SET_PROPERTY_VALUE(SelectedIndex, InValue);
		RefreshSelectionDetails();
	}
}

FText UVM_InventoryScreen::GetSelectedItemDisplayName() const
{
	return SelectedItemDisplayName;
}

void UVM_InventoryScreen::SetSelectedItemDisplayName(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SelectedItemDisplayName, InValue);
}

FText UVM_InventoryScreen::GetSelectedItemCountText() const
{
	return SelectedItemCountText;
}

void UVM_InventoryScreen::SetSelectedItemCountText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SelectedItemCountText, InValue);
}

FText UVM_InventoryScreen::GetSelectedItemScoreValueText() const
{
	return SelectedItemScoreValueText;
}

void UVM_InventoryScreen::SetSelectedItemScoreValueText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SelectedItemScoreValueText, InValue);
}

void UVM_InventoryScreen::RefreshSelectionDetails()
{
	if (!ItemEntries.IsValidIndex(SelectedIndex) || !ItemEntries[SelectedIndex])
	{
		SetSelectedItemDisplayName(INVTEXT("No item selected"));
		SetSelectedItemCountText(FText::GetEmpty());
		SetSelectedItemScoreValueText(FText::GetEmpty());
		return;
	}

	UVM_InventoryItemEntry* SelectedEntry = ItemEntries[SelectedIndex];
	SetSelectedItemDisplayName(SelectedEntry->GetDisplayName());
	SetSelectedItemCountText(SelectedEntry->GetCountText());
	SetSelectedItemScoreValueText(SelectedEntry->GetScoreValueText());
}

void UVM_InventoryScreen::RefreshFromStore()
{
	TArray<TObjectPtr<UVM_InventoryItemEntry>> NewEntries;

	if (InventoryStore.IsValid())
	{
		for (const FInventoryItemSnapshot& Snapshot : InventoryStore->GetItems())
		{
			UVM_InventoryItemEntry* EntryViewModel = NewObject<UVM_InventoryItemEntry>(this);
			EntryViewModel->SetItemId(Snapshot.ItemId);
			EntryViewModel->SetDisplayName(Snapshot.DisplayName);
			EntryViewModel->SetItemCount(Snapshot.Count);
			EntryViewModel->SetItemScoreValue(Snapshot.ScoreValue);
			EntryViewModel->SetCountText(FText::Format(INVTEXT("Count: {0}"), FText::AsNumber(Snapshot.Count)));
			EntryViewModel->SetScoreValueText(FText::Format(INVTEXT("Value: {0}"), FText::AsNumber(Snapshot.ScoreValue)));
			EntryViewModel->SetTintColor(Snapshot.TintColor);
			EntryViewModel->SetBillboardTexture(Snapshot.BillboardTexture.LoadSynchronous());
			NewEntries.Add(EntryViewModel);
		}
	}

	SetItemEntries(NewEntries);
	SetTotalItemCountText(FText::Format(INVTEXT("Total Items: {0}"), FText::AsNumber(InventoryStore.IsValid() ? InventoryStore->GetTotalItemCount() : 0)));
	SetTotalValueText(FText::Format(INVTEXT("Total Value: {0}"), FText::AsNumber(InventoryStore.IsValid() ? InventoryStore->GetTotalValue() : 0)));

	const int32 StoreSelectedIndex = InventoryStore.IsValid() ? InventoryStore->GetSelectedIndex() : INDEX_NONE;
	UE_MVVM_SET_PROPERTY_VALUE(SelectedIndex, StoreSelectedIndex);
	RefreshSelectionDetails();
}

void UVM_InventoryScreen::HandleStoreChanged(UUIStoreBase* ChangedStore)
{
	RefreshFromStore();
}
