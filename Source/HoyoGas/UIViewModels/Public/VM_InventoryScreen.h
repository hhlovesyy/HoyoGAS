#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VM_InventoryScreen.generated.h"

class UInventoryUIStore;
class UUIStoreBase;
class UVM_InventoryItemEntry;

UCLASS(BlueprintType)
class HOYOGAS_API UVM_InventoryScreen : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(UInventoryUIStore* InInventoryStore);
	void Teardown();

	const TArray<TObjectPtr<UVM_InventoryItemEntry>>& GetItemEntries() const;
	void SetItemEntries(const TArray<TObjectPtr<UVM_InventoryItemEntry>>& InEntries);

	FText GetTotalItemCountText() const;
	void SetTotalItemCountText(const FText& InValue);

	FText GetTotalValueText() const;
	void SetTotalValueText(const FText& InValue);

	int32 GetSelectedIndex() const;
	void SetSelectedIndex(int32 InValue);

	FText GetSelectedItemDisplayName() const;
	void SetSelectedItemDisplayName(const FText& InValue);

	FText GetSelectedItemCountText() const;
	void SetSelectedItemCountText(const FText& InValue);

	FText GetSelectedItemScoreValueText() const;
	void SetSelectedItemScoreValueText(const FText& InValue);

	void RefreshSelectionDetails();

private:
	void RefreshFromStore();
	void HandleStoreChanged(UUIStoreBase* ChangedStore);
protected:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UVM_InventoryItemEntry>> ItemEntries;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText TotalItemCountText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText TotalValueText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	int32 SelectedIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText SelectedItemDisplayName;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText SelectedItemCountText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText SelectedItemScoreValueText;

	TWeakObjectPtr<UInventoryUIStore> InventoryStore;
};
