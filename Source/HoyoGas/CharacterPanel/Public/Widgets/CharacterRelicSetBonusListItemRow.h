#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "CharacterRelicSetBonusListItemRow.generated.h"

namespace UE::FieldNotification
{
	struct FFieldId;
}

class UVM_CharacterRelicSetBonusEntry;

UCLASS()
class HOYOGAS_API UCharacterRelicSetBonusListItemRow : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnEntryReleased() override;

private:
	void BindViewModel(UVM_CharacterRelicSetBonusEntry* InViewModel);
	void RefreshVisuals();
	void HandleViewModelFieldChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId);
	FSlateFontInfo MakeFont(int32 Size) const;

	UPROPERTY(Transient)
	TObjectPtr<UVM_CharacterRelicSetBonusEntry> ViewModel;

	TSharedPtr<class SBorder> RowBorder;
	TSharedPtr<class STextBlock> SetNameTextBlock;
	TSharedPtr<class STextBlock> ActivationTextBlock;
};
