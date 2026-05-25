#pragma once

#include "CoreMinimal.h"
#include "Stores/MyUIStoreBase.h"
#include "Stores/MyUIStoreTypes.h"
#include "InventoryUIStore.generated.h"

class UInventoryComponent;

UCLASS(BlueprintType)
class HOYOGAS_API UInventoryUIStore : public UUIStoreBase
{
	GENERATED_BODY()

public:
	virtual void BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState) override;
	virtual void UnbindFromPlayerContext() override;

	void BindGameplaySources(UInventoryComponent* InInventoryComponent);
	void UnbindGameplaySources();
	void RebuildFromInventoryComponent();

	const TArray<FInventoryItemSnapshot>& GetItems() const;
	int32 GetTotalItemCount() const;
	int32 GetTotalValue() const;
	int32 GetSelectedIndex() const;
	FName GetSelectedItemId() const;
	const FInventoryItemSnapshot* GetSelectedItem() const;
	void SetSelectedIndex(int32 InSelectedIndex);

private:
	UFUNCTION()
	void HandleInventoryChanged();

	UPROPERTY(Transient)
	TWeakObjectPtr<UInventoryComponent> InventoryComponent;

	UPROPERTY(Transient)
	TArray<FInventoryItemSnapshot> Items;

	UPROPERTY(Transient)
	int32 TotalItemCount = 0;

	UPROPERTY(Transient)
	int32 TotalValue = 0;

	UPROPERTY(Transient)
	int32 SelectedIndex = INDEX_NONE;

	UPROPERTY(Transient)
	FName SelectedItemId = NAME_None;
};
