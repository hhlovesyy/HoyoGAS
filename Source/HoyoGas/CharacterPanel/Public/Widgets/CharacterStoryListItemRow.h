#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "CharacterStoryListItemRow.generated.h"

namespace UE::FieldNotification
{
	struct FFieldId;
}

class UVM_CharacterStoryEntry;

UCLASS()
class HOYOGAS_API UCharacterStoryListItemRow : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;
	virtual void NativeOnEntryReleased() override;

private:
	void RefreshVisuals();
	void BindViewModel(UVM_CharacterStoryEntry* InViewModel);
	void HandleViewModelFieldChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId);
	FSlateFontInfo MakeTitleFont() const;

	UPROPERTY(Transient)
	TObjectPtr<UVM_CharacterStoryEntry> ViewModel;

	TSharedPtr<class SBorder> RowBorder;
	TSharedPtr<class STextBlock> TitleTextBlock;
};
