#include "VM_InventoryItemEntry.h"

FName UVM_InventoryItemEntry::GetItemId() const
{
	return ItemId;
}

void UVM_InventoryItemEntry::SetItemId(FName InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ItemId, InValue);
}

FText UVM_InventoryItemEntry::GetDisplayName() const
{
	return DisplayName;
}

void UVM_InventoryItemEntry::SetDisplayName(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(DisplayName, InValue);
}

FText UVM_InventoryItemEntry::GetCountText() const
{
	return CountText;
}

void UVM_InventoryItemEntry::SetCountText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CountText, InValue);
}

FText UVM_InventoryItemEntry::GetScoreValueText() const
{
	return ScoreValueText;
}

void UVM_InventoryItemEntry::SetScoreValueText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ScoreValueText, InValue);
}

FLinearColor UVM_InventoryItemEntry::GetTintColor() const
{
	return TintColor;
}

void UVM_InventoryItemEntry::SetTintColor(const FLinearColor& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(TintColor, InValue);
}

UTexture2D* UVM_InventoryItemEntry::GetBillboardTexture() const
{
	return BillboardTexture;
}

void UVM_InventoryItemEntry::SetBillboardTexture(UTexture2D* InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(BillboardTexture, InValue);
}

int32 UVM_InventoryItemEntry::GetItemCount() const
{
	return ItemCount;
}

void UVM_InventoryItemEntry::SetItemCount(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ItemCount, InValue);
}

int32 UVM_InventoryItemEntry::GetItemScoreValue() const
{
	return ItemScoreValue;
}

void UVM_InventoryItemEntry::SetItemScoreValue(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ItemScoreValue, InValue);
}
