#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "OrigamiBirdPropListItemWidget.generated.h"

class UBorder;
class UImage;
class UTextBlock;
class UVM_OrigamiBirdPropEntry;

// 道具 ListView 的行 Widget。
// UE ListView 会复用行 Widget，所以这里在 NativeOnListItemObjectSet 里接收新的 Entry VM。
UCLASS()
class HOYOGAS_API UOrigamiBirdPropListItemWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;
	virtual void NativeOnEntryReleased() override;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UBorder> RowBorder;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> DisplayNameText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> CountText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> DescriptionText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> StackRuleText;

private:
	void EnsureDefaultVisualTree();
	void BindEntryViewModel(UVM_OrigamiBirdPropEntry* InEntryViewModel);
	void RefreshVisuals();

	UPROPERTY(Transient)
	TObjectPtr<UVM_OrigamiBirdPropEntry> CachedPropEntryViewModel;
};
