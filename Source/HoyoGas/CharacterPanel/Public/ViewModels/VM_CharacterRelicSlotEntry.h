#pragma once

#include "CoreMinimal.h"
#include "Data/HoyoRelicTypes.h"
#include "MVVMViewModelBase.h"
#include "VM_CharacterRelicSlotEntry.generated.h"

class UTexture2D;

// 遗器槽位列表项 VM：一个对象代表一个槽位，避免在 Screen VM 上写死 Head/Hands 等 6 组字段。
UCLASS(BlueprintType)
class HOYOGAS_API UVM_CharacterRelicSlotEntry : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	EHoyoRelicSlot GetSlot() const;
	void SetSlot(EHoyoRelicSlot InValue);

	FText GetSlotNameText() const;
	void SetSlotNameText(const FText& InValue);

	FText GetRelicNameText() const;
	void SetRelicNameText(const FText& InValue);

	FText GetSetNameText() const;
	void SetSetNameText(const FText& InValue);

	FText GetLevelText() const;
	void SetLevelText(const FText& InValue);

	FText GetMainAffixText() const;
	void SetMainAffixText(const FText& InValue);

	FText GetSubAffixSummaryText() const;
	void SetSubAffixSummaryText(const FText& InValue);

	UTexture2D* GetIconTexture() const;
	void SetIconTexture(UTexture2D* InValue);

	bool GetIsEquipped() const;
	void SetIsEquipped(bool bInValue);

protected:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	EHoyoRelicSlot Slot = EHoyoRelicSlot::Head;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText SlotNameText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText RelicNameText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText SetNameText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText LevelText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText MainAffixText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText SubAffixSummaryText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> IconTexture;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter = "GetIsEquipped", Setter = "SetIsEquipped", meta = (AllowPrivateAccess = "true"))
	bool bIsEquipped = false;
};
