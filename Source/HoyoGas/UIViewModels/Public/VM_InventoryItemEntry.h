#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VM_InventoryItemEntry.generated.h"

class UTexture2D;

UCLASS(BlueprintType)
class HOYOGAS_API UVM_InventoryItemEntry : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	FName GetItemId() const;
	void SetItemId(FName InValue);

	FText GetDisplayName() const;
	void SetDisplayName(const FText& InValue);

	FText GetCountText() const;
	void SetCountText(const FText& InValue);

	FText GetScoreValueText() const;
	void SetScoreValueText(const FText& InValue);

	FLinearColor GetTintColor() const;
	void SetTintColor(const FLinearColor& InValue);

	UTexture2D* GetBillboardTexture() const;
	void SetBillboardTexture(UTexture2D* InValue);

	int32 GetItemCount() const;
	void SetItemCount(int32 InValue);

	int32 GetItemScoreValue() const;
	void SetItemScoreValue(int32 InValue);

protected:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FName ItemId = NAME_None;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText DisplayName;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CountText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText ScoreValueText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FLinearColor TintColor = FLinearColor::White;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> BillboardTexture;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	int32 ItemCount = 0;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	int32 ItemScoreValue = 0;
};
