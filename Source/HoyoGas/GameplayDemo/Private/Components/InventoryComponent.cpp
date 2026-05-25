#include "Components/InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::AddItem(FName ItemId, int32 Count)
{
	if (ItemId.IsNone() || Count <= 0)
	{
		return;
	}

	int32& ExistingCount = ItemCounts.FindOrAdd(ItemId);
	ExistingCount += Count;
	OnInventoryChanged.Broadcast();
}

int32 UInventoryComponent::GetItemCount(FName ItemId) const
{
	if (const int32* FoundCount = ItemCounts.Find(ItemId))
	{
		return *FoundCount;
	}

	return 0;
}

void UInventoryComponent::GetAllItems(TArray<FInventoryItemStack>& OutItems) const
{
	OutItems.Reset();

	for (const TPair<FName, int32>& Pair : ItemCounts)
	{
		FInventoryItemStack Stack;
		Stack.ItemId = Pair.Key;
		Stack.Count = Pair.Value;
		OutItems.Add(Stack);
	}

	OutItems.Sort([](const FInventoryItemStack& A, const FInventoryItemStack& B)
	{
		return A.ItemId.LexicalLess(B.ItemId);
	});
}

int32 UInventoryComponent::GetTotalItemCount() const
{
	int32 TotalCount = 0;
	for (const TPair<FName, int32>& Pair : ItemCounts)
	{
		TotalCount += Pair.Value;
	}
	return TotalCount;
}
