#include "Stores/InventoryUIStore.h"

#include "Components/InventoryComponent.h"
#include "Data/ItemDefinitionRow.h"
#include "GameplayDemoStatics.h"
#include "HoyoGasPlayerState.h"

void UInventoryUIStore::BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState)
{
	Super::BindToPlayerContext(InPawn, InPlayerState);

	AHoyoGasPlayerState* HoyoGasPlayerState = Cast<AHoyoGasPlayerState>(InPlayerState);
	BindGameplaySources(HoyoGasPlayerState ? HoyoGasPlayerState->GetInventoryComponent() : nullptr);
}

void UInventoryUIStore::UnbindFromPlayerContext()
{
	UnbindGameplaySources();
	Super::UnbindFromPlayerContext();
}

void UInventoryUIStore::BindGameplaySources(UInventoryComponent* InInventoryComponent)
{
	if (InventoryComponent.Get() == InInventoryComponent)
	{
		RebuildFromInventoryComponent();
		return;
	}

	UnbindGameplaySources();
	InventoryComponent = InInventoryComponent;

	if (InventoryComponent.IsValid())
	{
		InventoryComponent->OnInventoryChanged.AddDynamic(this, &UInventoryUIStore::HandleInventoryChanged);
	}

	RebuildFromInventoryComponent();
}

void UInventoryUIStore::UnbindGameplaySources()
{
	if (InventoryComponent.IsValid())
	{
		InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UInventoryUIStore::HandleInventoryChanged);
	}

	InventoryComponent.Reset();
	Items.Reset();
	TotalItemCount = 0;
	TotalValue = 0;
	SelectedIndex = INDEX_NONE;
	SelectedItemId = NAME_None;
	BroadcastStoreChanged();
}

void UInventoryUIStore::RebuildFromInventoryComponent()
{
	Items.Reset();
	TotalItemCount = 0;
	TotalValue = 0;

	TArray<FInventoryItemStack> InventoryStacks;
	if (InventoryComponent.IsValid())
	{
		InventoryComponent->GetAllItems(InventoryStacks);
	}

	for (const FInventoryItemStack& Stack : InventoryStacks)
	{
		FItemDefinitionRow ItemRow;
		if (!UGameplayDemoStatics::TryGetItemDefinition(this, Stack.ItemId, ItemRow))
		{
			continue;
		}

		FInventoryItemSnapshot Snapshot;
		Snapshot.ItemId = Stack.ItemId;
		Snapshot.DisplayName = ItemRow.DisplayName;
		Snapshot.Count = Stack.Count;
		Snapshot.ScoreValue = ItemRow.ScoreValue;
		Snapshot.TintColor = ItemRow.TintColor;
		Snapshot.BillboardTexture = ItemRow.BillboardTexture;
		Items.Add(Snapshot);

		TotalItemCount += Stack.Count;
		TotalValue += Stack.Count * ItemRow.ScoreValue;
	}

	if (Items.IsEmpty())
	{
		SelectedIndex = INDEX_NONE;
		SelectedItemId = NAME_None;
	}
	else if (SelectedItemId != NAME_None)
	{
		const int32 FoundIndex = Items.IndexOfByPredicate([this](const FInventoryItemSnapshot& Snapshot)
		{
			return Snapshot.ItemId == SelectedItemId;
		});
		SelectedIndex = FoundIndex != INDEX_NONE ? FoundIndex : 0;
		SelectedItemId = Items[SelectedIndex].ItemId;
	}
	else
	{
		SelectedIndex = FMath::Clamp(SelectedIndex, 0, Items.Num() - 1);
		SelectedItemId = Items.IsValidIndex(SelectedIndex) ? Items[SelectedIndex].ItemId : Items[0].ItemId;
		if (!Items.IsValidIndex(SelectedIndex))
		{
			SelectedIndex = 0;
		}
	}

	BroadcastStoreChanged();
}

const TArray<FInventoryItemSnapshot>& UInventoryUIStore::GetItems() const
{
	return Items;
}

int32 UInventoryUIStore::GetTotalItemCount() const
{
	return TotalItemCount;
}

int32 UInventoryUIStore::GetTotalValue() const
{
	return TotalValue;
}

int32 UInventoryUIStore::GetSelectedIndex() const
{
	return SelectedIndex;
}

FName UInventoryUIStore::GetSelectedItemId() const
{
	return SelectedItemId;
}

const FInventoryItemSnapshot* UInventoryUIStore::GetSelectedItem() const
{
	return Items.IsValidIndex(SelectedIndex) ? &Items[SelectedIndex] : nullptr;
}

void UInventoryUIStore::SetSelectedIndex(int32 InSelectedIndex)
{
	const int32 NewIndex = Items.IsValidIndex(InSelectedIndex) ? InSelectedIndex : INDEX_NONE;
	if (SelectedIndex == NewIndex)
	{
		return;
	}

	SelectedIndex = NewIndex;
	SelectedItemId = Items.IsValidIndex(SelectedIndex) ? Items[SelectedIndex].ItemId : NAME_None;
	BroadcastStoreChanged();
}

void UInventoryUIStore::HandleInventoryChanged()
{
	RebuildFromInventoryComponent();
}
