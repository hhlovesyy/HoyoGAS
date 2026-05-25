#pragma once

#include "CoreMinimal.h"
#include "Data/HoyoRelicTypes.h"
#include "MVVMViewModelBase.h"
#include "VM_CharacterPanelScreen.generated.h"

class UCharacterPanelUIStore;
class UUIStoreBase;
class UVM_CharacterRelicSetBonusEntry;
class UVM_CharacterRelicSlotEntry;
class UVM_CharacterStoryEntry;

UCLASS(BlueprintType)
class HOYOGAS_API UVM_CharacterPanelScreen : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(UCharacterPanelUIStore* InCharacterPanelStore);
	void Teardown();

	int32 GetSelectedTabIndex() const;
	void SetSelectedTabIndex(int32 InValue);

	FText GetSelectedTabLabel() const;
	void SetSelectedTabLabel(const FText& InValue);

	FText GetCharacterNameText() const;
	void SetCharacterNameText(const FText& InValue);

	FText GetCharacterTitleText() const;
	void SetCharacterTitleText(const FText& InValue);

	FText GetCharacterLevelText() const;
	void SetCharacterLevelText(const FText& InValue);

	FText GetCharacterPathText() const;
	void SetCharacterPathText(const FText& InValue);

	FText GetCharacterFactionText() const;
	void SetCharacterFactionText(const FText& InValue);

	FText GetCharacterElementText() const;
	void SetCharacterElementText(const FText& InValue);

	FText GetCharacterRarityText() const;
	void SetCharacterRarityText(const FText& InValue);

	FText GetCharacterFirstMetDateText() const;
	void SetCharacterFirstMetDateText(const FText& InValue);

	FText GetCharacterShortBioText() const;
	void SetCharacterShortBioText(const FText& InValue);

	FText GetSelectedStoryTitleText() const;
	void SetSelectedStoryTitleText(const FText& InValue);

	FText GetSelectedStoryBodyText() const;
	void SetSelectedStoryBodyText(const FText& InValue);

	const TArray<TObjectPtr<UVM_CharacterStoryEntry>>& GetStoryEntries() const;
	void SetStoryEntries(const TArray<TObjectPtr<UVM_CharacterStoryEntry>>& InEntries);

	int32 GetSelectedStoryIndex() const;
	void SetSelectedStoryIndex(int32 InValue);

	const TArray<TObjectPtr<UVM_CharacterRelicSlotEntry>>& GetRelicSlotEntries() const;
	void SetRelicSlotEntries(const TArray<TObjectPtr<UVM_CharacterRelicSlotEntry>>& InEntries);

	const TArray<TObjectPtr<UVM_CharacterRelicSetBonusEntry>>& GetRelicSetBonusEntries() const;
	void SetRelicSetBonusEntries(const TArray<TObjectPtr<UVM_CharacterRelicSetBonusEntry>>& InEntries);

	FText GetRelicDevelopmentStatusText() const;
	void SetRelicDevelopmentStatusText(const FText& InValue);

	bool GetCanUseRelicDevelopmentActions() const;
	void SetCanUseRelicDevelopmentActions(bool bInValue);

	void GrantRelicDevelopmentLoadout();
	void SelectRelicSlot(EHoyoRelicSlot Slot);
	void ToggleSelectedRelicDetail();

	FText GetRelicCharacterStatsText() const;
	void SetRelicCharacterStatsText(const FText& InValue);

	FText GetSelectedRelicNameText() const;
	void SetSelectedRelicNameText(const FText& InValue);

	FText GetSelectedRelicMetaText() const;
	void SetSelectedRelicMetaText(const FText& InValue);

	FText GetSelectedRelicMainAffixText() const;
	void SetSelectedRelicMainAffixText(const FText& InValue);

	FText GetSelectedRelicDetailText() const;
	void SetSelectedRelicDetailText(const FText& InValue);

	FText GetRelicSetSummaryText() const;
	void SetRelicSetSummaryText(const FText& InValue);

	FText GetRelicDetailButtonText() const;
	void SetRelicDetailButtonText(const FText& InValue);

private:
	void RefreshFromStore();
	void RefreshSelectedStory();
	void RefreshRelicsFromStore();
	void RefreshSelectedRelicFromStore();
	void HandleStoreChanged(UUIStoreBase* ChangedStore);
	UVM_CharacterRelicSlotEntry* BuildRelicSlotEntry(EHoyoRelicSlot Slot);
	FText BuildAffixText(const FHoyoRelicAffixInstance& AffixInstance) const;
	FText BuildRelicStatsSummaryText() const;
	FText BuildRelicSetSummaryText() const;
	FText GetRelicSlotDisplayName(EHoyoRelicSlot Slot) const;

protected:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	int32 SelectedTabIndex = 0;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText SelectedTabLabel;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CharacterNameText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CharacterTitleText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CharacterLevelText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CharacterPathText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CharacterFactionText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CharacterElementText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CharacterRarityText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CharacterFirstMetDateText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CharacterShortBioText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText SelectedStoryTitleText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText SelectedStoryBodyText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UVM_CharacterStoryEntry>> StoryEntries;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	int32 SelectedStoryIndex = 0;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UVM_CharacterRelicSlotEntry>> RelicSlotEntries;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UVM_CharacterRelicSetBonusEntry>> RelicSetBonusEntries;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText RelicDevelopmentStatusText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter = "GetCanUseRelicDevelopmentActions", Setter = "SetCanUseRelicDevelopmentActions", meta = (AllowPrivateAccess = "true"))
	bool bCanUseRelicDevelopmentActions = false;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText RelicCharacterStatsText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText SelectedRelicNameText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText SelectedRelicMetaText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText SelectedRelicMainAffixText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText SelectedRelicDetailText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText RelicSetSummaryText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText RelicDetailButtonText;

	UPROPERTY(Transient)
	EHoyoRelicSlot SelectedRelicSlot = EHoyoRelicSlot::Head;

	UPROPERTY(Transient)
	bool bShowSelectedRelicDetail = false;

	TWeakObjectPtr<UCharacterPanelUIStore> CharacterPanelStore;
};
