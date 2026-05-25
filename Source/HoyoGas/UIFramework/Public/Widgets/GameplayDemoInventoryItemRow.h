#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "GameplayDemoInventoryItemRow.generated.h"

class UBorder;
class UImage;
class UTextBlock;
class UVM_InventoryItemEntry;

UCLASS()
class HOYOGAS_API UGameplayDemoInventoryItemRow : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	void ApplyItemTexture(class UTexture2D* InTexture);

protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;
	virtual void NativeOnEntryReleased() override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UVM_InventoryItemEntry> ViewModel;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UBorder> RowBorder;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> ItemImage;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> CountText;
};
