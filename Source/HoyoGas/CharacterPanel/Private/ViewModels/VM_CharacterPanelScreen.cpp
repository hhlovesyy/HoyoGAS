#include "ViewModels/VM_CharacterPanelScreen.h"

#include "Data/CharacterDefinitionRow.h"
#include "Stores/CharacterPanelUIStore.h"
#include "ViewModels/VM_CharacterRelicSetBonusEntry.h"
#include "ViewModels/VM_CharacterRelicSlotEntry.h"
#include "ViewModels/VM_CharacterStoryEntry.h"

void UVM_CharacterPanelScreen::Initialize(UCharacterPanelUIStore* InCharacterPanelStore)
{
	if (CharacterPanelStore.Get() != InCharacterPanelStore)
	{
		if (CharacterPanelStore.IsValid())
		{
			CharacterPanelStore->OnStoreChanged().RemoveAll(this);
		}

		CharacterPanelStore = InCharacterPanelStore;
		if (CharacterPanelStore.IsValid())
		{
			CharacterPanelStore->OnStoreChanged().AddUObject(this, &UVM_CharacterPanelScreen::HandleStoreChanged);
		}
	}

	RefreshFromStore();
}

void UVM_CharacterPanelScreen::Teardown()
{
	if (CharacterPanelStore.IsValid())
	{
		CharacterPanelStore->OnStoreChanged().RemoveAll(this);
		CharacterPanelStore.Reset();
	}
}

int32 UVM_CharacterPanelScreen::GetSelectedTabIndex() const
{
	return SelectedTabIndex;
}

void UVM_CharacterPanelScreen::SetSelectedTabIndex(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SelectedTabIndex, InValue);
}

FText UVM_CharacterPanelScreen::GetSelectedTabLabel() const
{
	return SelectedTabLabel;
}

void UVM_CharacterPanelScreen::SetSelectedTabLabel(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SelectedTabLabel, InValue);
}

FText UVM_CharacterPanelScreen::GetCharacterNameText() const
{
	return CharacterNameText;
}

void UVM_CharacterPanelScreen::SetCharacterNameText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CharacterNameText, InValue);
}

FText UVM_CharacterPanelScreen::GetCharacterTitleText() const
{
	return CharacterTitleText;
}

void UVM_CharacterPanelScreen::SetCharacterTitleText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CharacterTitleText, InValue);
}

FText UVM_CharacterPanelScreen::GetCharacterLevelText() const
{
	return CharacterLevelText;
}

void UVM_CharacterPanelScreen::SetCharacterLevelText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CharacterLevelText, InValue);
}

FText UVM_CharacterPanelScreen::GetCharacterPathText() const
{
	return CharacterPathText;
}

void UVM_CharacterPanelScreen::SetCharacterPathText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CharacterPathText, InValue);
}

FText UVM_CharacterPanelScreen::GetCharacterFactionText() const
{
	return CharacterFactionText;
}

void UVM_CharacterPanelScreen::SetCharacterFactionText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CharacterFactionText, InValue);
}

FText UVM_CharacterPanelScreen::GetCharacterElementText() const
{
	return CharacterElementText;
}

void UVM_CharacterPanelScreen::SetCharacterElementText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CharacterElementText, InValue);
}

FText UVM_CharacterPanelScreen::GetCharacterRarityText() const
{
	return CharacterRarityText;
}

void UVM_CharacterPanelScreen::SetCharacterRarityText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CharacterRarityText, InValue);
}

FText UVM_CharacterPanelScreen::GetCharacterFirstMetDateText() const
{
	return CharacterFirstMetDateText;
}

void UVM_CharacterPanelScreen::SetCharacterFirstMetDateText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CharacterFirstMetDateText, InValue);
}

FText UVM_CharacterPanelScreen::GetCharacterShortBioText() const
{
	return CharacterShortBioText;
}

void UVM_CharacterPanelScreen::SetCharacterShortBioText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CharacterShortBioText, InValue);
}

FText UVM_CharacterPanelScreen::GetSelectedStoryTitleText() const
{
	return SelectedStoryTitleText;
}

void UVM_CharacterPanelScreen::SetSelectedStoryTitleText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SelectedStoryTitleText, InValue);
}

FText UVM_CharacterPanelScreen::GetSelectedStoryBodyText() const
{
	return SelectedStoryBodyText;
}

void UVM_CharacterPanelScreen::SetSelectedStoryBodyText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SelectedStoryBodyText, InValue);
}

const TArray<TObjectPtr<UVM_CharacterStoryEntry>>& UVM_CharacterPanelScreen::GetStoryEntries() const
{
	return StoryEntries;
}

void UVM_CharacterPanelScreen::SetStoryEntries(const TArray<TObjectPtr<UVM_CharacterStoryEntry>>& InEntries)
{
	StoryEntries = InEntries;
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(StoryEntries);
}

int32 UVM_CharacterPanelScreen::GetSelectedStoryIndex() const
{
	return SelectedStoryIndex;
}

void UVM_CharacterPanelScreen::SetSelectedStoryIndex(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SelectedStoryIndex, StoryEntries.IsValidIndex(InValue) ? InValue : INDEX_NONE);
	for (UVM_CharacterStoryEntry* Entry : StoryEntries)
	{
		if (Entry)
		{
			Entry->SetIsSelected(Entry->GetStoryIndex() == SelectedStoryIndex);
		}
	}
	RefreshSelectedStory();
}

const TArray<TObjectPtr<UVM_CharacterRelicSlotEntry>>& UVM_CharacterPanelScreen::GetRelicSlotEntries() const
{
	return RelicSlotEntries;
}

void UVM_CharacterPanelScreen::SetRelicSlotEntries(const TArray<TObjectPtr<UVM_CharacterRelicSlotEntry>>& InEntries)
{
	RelicSlotEntries = InEntries;
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(RelicSlotEntries);
}

const TArray<TObjectPtr<UVM_CharacterRelicSetBonusEntry>>& UVM_CharacterPanelScreen::GetRelicSetBonusEntries() const
{
	return RelicSetBonusEntries;
}

void UVM_CharacterPanelScreen::SetRelicSetBonusEntries(const TArray<TObjectPtr<UVM_CharacterRelicSetBonusEntry>>& InEntries)
{
	RelicSetBonusEntries = InEntries;
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(RelicSetBonusEntries);
}

FText UVM_CharacterPanelScreen::GetRelicDevelopmentStatusText() const
{
	return RelicDevelopmentStatusText;
}

void UVM_CharacterPanelScreen::SetRelicDevelopmentStatusText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(RelicDevelopmentStatusText, InValue);
}

bool UVM_CharacterPanelScreen::GetCanUseRelicDevelopmentActions() const
{
	return bCanUseRelicDevelopmentActions;
}

void UVM_CharacterPanelScreen::SetCanUseRelicDevelopmentActions(bool bInValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bCanUseRelicDevelopmentActions, bInValue);
}

void UVM_CharacterPanelScreen::SelectRelicSlot(EHoyoRelicSlot Slot)
{
	SelectedRelicSlot = Slot;
	bShowSelectedRelicDetail = false;
	RefreshSelectedRelicFromStore();
}

void UVM_CharacterPanelScreen::ToggleSelectedRelicDetail()
{
	bShowSelectedRelicDetail = !bShowSelectedRelicDetail;
	RefreshSelectedRelicFromStore();
}

FText UVM_CharacterPanelScreen::GetRelicCharacterStatsText() const
{
	return RelicCharacterStatsText;
}

void UVM_CharacterPanelScreen::SetRelicCharacterStatsText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(RelicCharacterStatsText, InValue);
}

FText UVM_CharacterPanelScreen::GetSelectedRelicNameText() const
{
	return SelectedRelicNameText;
}

void UVM_CharacterPanelScreen::SetSelectedRelicNameText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SelectedRelicNameText, InValue);
}

FText UVM_CharacterPanelScreen::GetSelectedRelicMetaText() const
{
	return SelectedRelicMetaText;
}

void UVM_CharacterPanelScreen::SetSelectedRelicMetaText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SelectedRelicMetaText, InValue);
}

FText UVM_CharacterPanelScreen::GetSelectedRelicMainAffixText() const
{
	return SelectedRelicMainAffixText;
}

void UVM_CharacterPanelScreen::SetSelectedRelicMainAffixText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SelectedRelicMainAffixText, InValue);
}

FText UVM_CharacterPanelScreen::GetSelectedRelicDetailText() const
{
	return SelectedRelicDetailText;
}

void UVM_CharacterPanelScreen::SetSelectedRelicDetailText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SelectedRelicDetailText, InValue);
}

FText UVM_CharacterPanelScreen::GetRelicSetSummaryText() const
{
	return RelicSetSummaryText;
}

void UVM_CharacterPanelScreen::SetRelicSetSummaryText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(RelicSetSummaryText, InValue);
}

FText UVM_CharacterPanelScreen::GetRelicDetailButtonText() const
{
	return RelicDetailButtonText;
}

void UVM_CharacterPanelScreen::SetRelicDetailButtonText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(RelicDetailButtonText, InValue);
}

void UVM_CharacterPanelScreen::GrantRelicDevelopmentLoadout()
{
	if (!CharacterPanelStore.IsValid())
	{
		SetRelicDevelopmentStatusText(NSLOCTEXT("CharacterPanel", "RelicDevNoStore", "Character store is not available."));
		return;
	}

	FText StatusText;
	CharacterPanelStore->GrantRelicDevelopmentLoadoutForCurrentCharacter(StatusText);
	SetRelicDevelopmentStatusText(StatusText);
	RefreshRelicsFromStore();
}

void UVM_CharacterPanelScreen::RefreshFromStore()
{
	if (!CharacterPanelStore.IsValid() || !CharacterPanelStore->HasCurrentCharacter())
	{
		SetCharacterNameText(NSLOCTEXT("CharacterPanel", "NoCharacterName", "No Character"));
		SetCharacterTitleText(FText::GetEmpty());
		SetCharacterLevelText(FText::GetEmpty());
		SetCharacterPathText(FText::GetEmpty());
		SetCharacterFactionText(FText::GetEmpty());
		SetCharacterElementText(FText::GetEmpty());
		SetCharacterRarityText(FText::GetEmpty());
		SetCharacterFirstMetDateText(FText::GetEmpty());
		SetCharacterShortBioText(FText::GetEmpty());
		SetSelectedStoryTitleText(FText::GetEmpty());
		SetSelectedStoryBodyText(FText::GetEmpty());
		TArray<TObjectPtr<UVM_CharacterStoryEntry>> EmptyStoryEntries;
		SetStoryEntries(EmptyStoryEntries);
		TArray<TObjectPtr<UVM_CharacterRelicSlotEntry>> EmptyRelicSlotEntries;
		SetRelicSlotEntries(EmptyRelicSlotEntries);
		TArray<TObjectPtr<UVM_CharacterRelicSetBonusEntry>> EmptyRelicSetBonusEntries;
		SetRelicSetBonusEntries(EmptyRelicSetBonusEntries);
		SetCanUseRelicDevelopmentActions(false);
		SetRelicCharacterStatsText(FText::GetEmpty());
		SetSelectedRelicNameText(FText::GetEmpty());
		SetSelectedRelicMetaText(FText::GetEmpty());
		SetSelectedRelicMainAffixText(FText::GetEmpty());
		SetSelectedRelicDetailText(FText::GetEmpty());
		SetRelicSetSummaryText(FText::GetEmpty());
		SetRelicDetailButtonText(FText::GetEmpty());
		UE_MVVM_SET_PROPERTY_VALUE(SelectedStoryIndex, INDEX_NONE);
		return;
	}

	const FCharacterDefinitionRow& Character = CharacterPanelStore->GetCurrentCharacter();
	SetCharacterNameText(Character.DisplayName);
	SetCharacterTitleText(Character.Title);
	SetCharacterLevelText(FText::Format(NSLOCTEXT("CharacterPanel", "LevelFormat", "Lv. {0} / {1}"), FText::AsNumber(Character.DefaultLevel), FText::AsNumber(Character.MaxLevel)));
	SetCharacterPathText(FText::Format(NSLOCTEXT("CharacterPanel", "PathFormat", "Path: {0}"), Character.PathName));
	SetCharacterFactionText(FText::Format(NSLOCTEXT("CharacterPanel", "FactionFormat", "Faction: {0}"), Character.FactionName));
	SetCharacterElementText(FText::Format(NSLOCTEXT("CharacterPanel", "ElementFormat", "Element: {0}"), Character.ElementName));
	SetCharacterRarityText(FText::Format(NSLOCTEXT("CharacterPanel", "RarityFormat", "{0}-Star"), FText::AsNumber(Character.Rarity)));
	SetCharacterFirstMetDateText(Character.FirstMetDateText);
	SetCharacterShortBioText(Character.ShortBio);

	TArray<TObjectPtr<UVM_CharacterStoryEntry>> NewStoryEntries;
	for (int32 StoryIndex = 0; StoryIndex < Character.Stories.Num(); ++StoryIndex)
	{
		const FCharacterStoryEntry& Story = Character.Stories[StoryIndex];
		UVM_CharacterStoryEntry* Entry = NewObject<UVM_CharacterStoryEntry>(this);
		Entry->SetStoryId(Story.StoryId);
		Entry->SetStoryIndex(StoryIndex);
		Entry->SetTitleText(Story.Title);
		Entry->SetIsUnlocked(CharacterPanelStore->IsStoryUnlocked(Story));
		NewStoryEntries.Add(Entry);
	}

	SetStoryEntries(NewStoryEntries);
	if (!StoryEntries.IsValidIndex(SelectedStoryIndex))
	{
		UE_MVVM_SET_PROPERTY_VALUE(SelectedStoryIndex, StoryEntries.IsEmpty() ? INDEX_NONE : 0);
	}
	for (UVM_CharacterStoryEntry* Entry : StoryEntries)
	{
		if (Entry)
		{
			Entry->SetIsSelected(Entry->GetStoryIndex() == SelectedStoryIndex);
		}
	}
	RefreshSelectedStory();
	RefreshRelicsFromStore();
}

void UVM_CharacterPanelScreen::RefreshSelectedStory()
{
	if (!CharacterPanelStore.IsValid() || !CharacterPanelStore->HasCurrentCharacter())
	{
		SetSelectedStoryTitleText(FText::GetEmpty());
		SetSelectedStoryBodyText(FText::GetEmpty());
		return;
	}

	const FCharacterDefinitionRow& Character = CharacterPanelStore->GetCurrentCharacter();
	const FCharacterStoryEntry* Story = Character.Stories.IsValidIndex(SelectedStoryIndex) ? &Character.Stories[SelectedStoryIndex] : nullptr;
	if (!Story)
	{
		SetSelectedStoryTitleText(FText::GetEmpty());
		SetSelectedStoryBodyText(FText::GetEmpty());
		return;
	}

	SetSelectedStoryTitleText(Story->Title);
	SetSelectedStoryBodyText(CharacterPanelStore->IsStoryUnlocked(*Story)
		? Story->RichTextBody
		: CharacterPanelStore->GetStoryLockedReason(*Story));
}

void UVM_CharacterPanelScreen::RefreshRelicsFromStore()
{
	TArray<TObjectPtr<UVM_CharacterRelicSlotEntry>> NewSlotEntries;
	NewSlotEntries.Add(BuildRelicSlotEntry(EHoyoRelicSlot::Head));
	NewSlotEntries.Add(BuildRelicSlotEntry(EHoyoRelicSlot::Hands));
	NewSlotEntries.Add(BuildRelicSlotEntry(EHoyoRelicSlot::Body));
	NewSlotEntries.Add(BuildRelicSlotEntry(EHoyoRelicSlot::Feet));
	NewSlotEntries.Add(BuildRelicSlotEntry(EHoyoRelicSlot::PlanarSphere));
	NewSlotEntries.Add(BuildRelicSlotEntry(EHoyoRelicSlot::LinkRope));
	SetRelicSlotEntries(NewSlotEntries);

	TArray<TObjectPtr<UVM_CharacterRelicSetBonusEntry>> NewSetBonusEntries;
	if (CharacterPanelStore.IsValid())
	{
		TArray<FHoyoRelicSetActivation> Activations;
		CharacterPanelStore->GetActiveRelicSetBonuses(Activations);
		for (const FHoyoRelicSetActivation& Activation : Activations)
		{
			FHoyoRelicSetDefinitionRow SetDefinition;
			const bool bHasSetDefinition = CharacterPanelStore->GetRelicSetDefinition(Activation.SetId, SetDefinition);

			UVM_CharacterRelicSetBonusEntry* Entry = NewObject<UVM_CharacterRelicSetBonusEntry>(this);
			Entry->SetSetId(Activation.SetId);
			Entry->SetSetNameText(bHasSetDefinition ? SetDefinition.DisplayName : FText::FromName(Activation.SetId));

			TArray<FText> ActiveCounts;
			for (const int32 Count : Activation.ActiveBonusPieceCounts)
			{
				ActiveCounts.Add(FText::AsNumber(Count));
			}

			const FText ActiveText = ActiveCounts.IsEmpty()
				? NSLOCTEXT("CharacterPanel", "RelicSetNoBonusActive", "No set bonus active")
				: FText::Format(NSLOCTEXT("CharacterPanel", "RelicSetBonusActiveFormat", "Active: {0}-piece"), FText::Join(FText::FromString(TEXT("/")), ActiveCounts));

			Entry->SetActivationText(FText::Format(NSLOCTEXT("CharacterPanel", "RelicSetActivationFormat", "{0} equipped. {1}"), FText::AsNumber(Activation.EquippedPieceCount), ActiveText));
			NewSetBonusEntries.Add(Entry);
		}
	}
	SetRelicSetBonusEntries(NewSetBonusEntries);

	SetCanUseRelicDevelopmentActions(CharacterPanelStore.IsValid() && CharacterPanelStore->IsRelicDevelopmentActionEnabled());
	SetRelicCharacterStatsText(BuildRelicStatsSummaryText());
	SetRelicSetSummaryText(BuildRelicSetSummaryText());
	RefreshSelectedRelicFromStore();
}

void UVM_CharacterPanelScreen::RefreshSelectedRelicFromStore()
{
	SetRelicDetailButtonText(bShowSelectedRelicDetail
		? NSLOCTEXT("CharacterPanel", "RelicDetailCollapse", "收起详情")
		: NSLOCTEXT("CharacterPanel", "RelicDetailExpand", "展开详情"));

	FHoyoRelicInstance RelicInstance;
	if (!CharacterPanelStore.IsValid() || !CharacterPanelStore->GetEquippedRelicInSlot(SelectedRelicSlot, RelicInstance))
	{
		SetSelectedRelicNameText(NSLOCTEXT("CharacterPanel", "SelectedRelicEmpty", "未装备"));
		SetSelectedRelicMetaText(GetRelicSlotDisplayName(SelectedRelicSlot));
		SetSelectedRelicMainAffixText(FText::GetEmpty());
		SetSelectedRelicDetailText(NSLOCTEXT("CharacterPanel", "SelectedRelicEmptyHint", "选择一个已装备遗器查看词条。"));
		return;
	}

	FHoyoRelicDefinitionRow RelicDefinition;
	const bool bHasRelicDefinition = CharacterPanelStore->GetRelicDefinition(RelicInstance.RelicDefinitionId, RelicDefinition);
	FHoyoRelicSetDefinitionRow SetDefinition;
	const bool bHasSetDefinition = CharacterPanelStore->GetRelicSetDefinition(RelicInstance.SetId, SetDefinition);

	SetSelectedRelicNameText(bHasRelicDefinition ? RelicDefinition.DisplayName : FText::FromName(RelicInstance.RelicDefinitionId));
	SetSelectedRelicMetaText(FText::Format(
		NSLOCTEXT("CharacterPanel", "SelectedRelicMetaFormat", "{0}  {1}  +{2}"),
		GetRelicSlotDisplayName(RelicInstance.Slot),
		bHasSetDefinition ? SetDefinition.DisplayName : FText::FromName(RelicInstance.SetId),
		FText::AsNumber(RelicInstance.Level)));
	SetSelectedRelicMainAffixText(BuildAffixText(RelicInstance.MainAffix));

	TArray<FText> DetailLines;
	if (bShowSelectedRelicDetail)
	{
		DetailLines.Add(NSLOCTEXT("CharacterPanel", "SelectedRelicSubAffixHeader", "副词条"));
		for (const FHoyoRelicAffixInstance& SubAffix : RelicInstance.SubAffixes)
		{
			DetailLines.Add(BuildAffixText(SubAffix));
		}
	}
	else
	{
		DetailLines.Add(NSLOCTEXT("CharacterPanel", "SelectedRelicCompactHint", "点击展开详情查看副词条。"));
	}

	SetSelectedRelicDetailText(FText::Join(FText::FromString(TEXT("\n")), DetailLines));
}

void UVM_CharacterPanelScreen::HandleStoreChanged(UUIStoreBase* ChangedStore)
{
	(void)ChangedStore;
	RefreshFromStore();
}

UVM_CharacterRelicSlotEntry* UVM_CharacterPanelScreen::BuildRelicSlotEntry(EHoyoRelicSlot Slot)
{
	UVM_CharacterRelicSlotEntry* Entry = NewObject<UVM_CharacterRelicSlotEntry>(this);
	Entry->SetSlot(Slot);
	Entry->SetSlotNameText(GetRelicSlotDisplayName(Slot));

	FHoyoRelicInstance RelicInstance;
	if (!CharacterPanelStore.IsValid() || !CharacterPanelStore->GetEquippedRelicInSlot(Slot, RelicInstance))
	{
		Entry->SetIsEquipped(false);
		Entry->SetRelicNameText(NSLOCTEXT("CharacterPanel", "RelicSlotEmpty", "Empty"));
		Entry->SetSetNameText(FText::GetEmpty());
		Entry->SetLevelText(FText::GetEmpty());
		Entry->SetMainAffixText(FText::GetEmpty());
		Entry->SetSubAffixSummaryText(FText::GetEmpty());
		return Entry;
	}

	FHoyoRelicDefinitionRow RelicDefinition;
	const bool bHasRelicDefinition = CharacterPanelStore->GetRelicDefinition(RelicInstance.RelicDefinitionId, RelicDefinition);
	FHoyoRelicSetDefinitionRow SetDefinition;
	const bool bHasSetDefinition = CharacterPanelStore->GetRelicSetDefinition(RelicInstance.SetId, SetDefinition);

	Entry->SetIsEquipped(true);
	Entry->SetRelicNameText(bHasRelicDefinition ? RelicDefinition.DisplayName : FText::FromName(RelicInstance.RelicDefinitionId));
	Entry->SetSetNameText(bHasSetDefinition ? SetDefinition.DisplayName : FText::FromName(RelicInstance.SetId));
	Entry->SetLevelText(FText::Format(NSLOCTEXT("CharacterPanel", "RelicLevelFormat", "+{0}"), FText::AsNumber(RelicInstance.Level)));
	Entry->SetMainAffixText(BuildAffixText(RelicInstance.MainAffix));
	Entry->SetIconTexture(bHasRelicDefinition ? RelicDefinition.Icon.LoadSynchronous() : nullptr);

	TArray<FText> SubAffixTexts;
	for (const FHoyoRelicAffixInstance& SubAffix : RelicInstance.SubAffixes)
	{
		SubAffixTexts.Add(BuildAffixText(SubAffix));
	}
	Entry->SetSubAffixSummaryText(FText::Join(FText::FromString(TEXT("\n")), SubAffixTexts));
	return Entry;
}

FText UVM_CharacterPanelScreen::BuildAffixText(const FHoyoRelicAffixInstance& AffixInstance) const
{
	if (AffixInstance.AffixId.IsNone())
	{
		return FText::GetEmpty();
	}

	FHoyoRelicAffixDefinitionRow AffixDefinition;
	const bool bHasAffixDefinition = CharacterPanelStore.IsValid() && CharacterPanelStore->GetRelicAffixDefinition(AffixInstance.AffixId, AffixDefinition);
	const FText AffixName = bHasAffixDefinition ? AffixDefinition.DisplayName : FText::FromName(AffixInstance.AffixId);
	const bool bPercent = bHasAffixDefinition && AffixDefinition.ValueType == EHoyoRelicStatValueType::Percent;
	const float DisplayValue = bPercent ? AffixInstance.Value * 100.0f : AffixInstance.Value;
	FNumberFormattingOptions NumberFormatOptions;
	NumberFormatOptions.MinimumFractionalDigits = bPercent ? 1 : 0;
	NumberFormatOptions.MaximumFractionalDigits = bPercent ? 1 : 0;

	return FText::Format(
		bPercent
			? NSLOCTEXT("CharacterPanel", "RelicAffixPercentFormat", "{0} +{1}%")
			: NSLOCTEXT("CharacterPanel", "RelicAffixFlatFormat", "{0} +{1}"),
		AffixName,
		FText::AsNumber(DisplayValue, &NumberFormatOptions));
}

FText UVM_CharacterPanelScreen::BuildRelicStatsSummaryText() const
{
	if (!CharacterPanelStore.IsValid())
	{
		return FText::GetEmpty();
	}

	TMap<FName, float> Totals;
	const EHoyoRelicSlot Slots[] =
	{
		EHoyoRelicSlot::Head,
		EHoyoRelicSlot::Hands,
		EHoyoRelicSlot::Body,
		EHoyoRelicSlot::Feet,
		EHoyoRelicSlot::PlanarSphere,
		EHoyoRelicSlot::LinkRope
	};

	for (const EHoyoRelicSlot Slot : Slots)
	{
		FHoyoRelicInstance RelicInstance;
		if (!CharacterPanelStore->GetEquippedRelicInSlot(Slot, RelicInstance))
		{
			continue;
		}

		Totals.FindOrAdd(RelicInstance.MainAffix.AffixId) += RelicInstance.MainAffix.Value;
		for (const FHoyoRelicAffixInstance& SubAffix : RelicInstance.SubAffixes)
		{
			Totals.FindOrAdd(SubAffix.AffixId) += SubAffix.Value;
		}
	}

	TArray<FText> Lines;
	Lines.Add(NSLOCTEXT("CharacterPanel", "RelicStatsPreviewHeader", "遗器属性预览（尚未接入 GAS）"));
	for (const TPair<FName, float>& Pair : Totals)
	{
		FHoyoRelicAffixDefinitionRow AffixDefinition;
		const bool bHasDefinition = CharacterPanelStore->GetRelicAffixDefinition(Pair.Key, AffixDefinition);
		const FText AffixName = bHasDefinition ? AffixDefinition.DisplayName : FText::FromName(Pair.Key);
		const bool bPercent = bHasDefinition && AffixDefinition.ValueType == EHoyoRelicStatValueType::Percent;
		const float DisplayValue = bPercent ? Pair.Value * 100.0f : Pair.Value;
		FNumberFormattingOptions NumberFormatOptions;
		NumberFormatOptions.MinimumFractionalDigits = bPercent ? 1 : 0;
		NumberFormatOptions.MaximumFractionalDigits = bPercent ? 1 : 0;
		Lines.Add(FText::Format(
			bPercent
				? NSLOCTEXT("CharacterPanel", "RelicStatsPercentLine", "{0} +{1}%")
				: NSLOCTEXT("CharacterPanel", "RelicStatsFlatLine", "{0} +{1}"),
			AffixName,
			FText::AsNumber(DisplayValue, &NumberFormatOptions)));
	}

	return FText::Join(FText::FromString(TEXT("\n")), Lines);
}

FText UVM_CharacterPanelScreen::BuildRelicSetSummaryText() const
{
	if (!CharacterPanelStore.IsValid())
	{
		return FText::GetEmpty();
	}

	TArray<FHoyoRelicSetActivation> Activations;
	CharacterPanelStore->GetActiveRelicSetBonuses(Activations);

	TArray<FText> Lines;
	for (const FHoyoRelicSetActivation& Activation : Activations)
	{
		FHoyoRelicSetDefinitionRow SetDefinition;
		const bool bHasSetDefinition = CharacterPanelStore->GetRelicSetDefinition(Activation.SetId, SetDefinition);
		Lines.Add(FText::Format(
			NSLOCTEXT("CharacterPanel", "RelicSetSummaryHeader", "{0}：{1} 件"),
			bHasSetDefinition ? SetDefinition.DisplayName : FText::FromName(Activation.SetId),
			FText::AsNumber(Activation.EquippedPieceCount)));

		if (bHasSetDefinition)
		{
			for (const FHoyoRelicSetBonusDefinition& Bonus : SetDefinition.Bonuses)
			{
				if (Activation.ActiveBonusPieceCounts.Contains(Bonus.RequiredPieceCount))
				{
					Lines.Add(FText::Format(
						NSLOCTEXT("CharacterPanel", "RelicSetSummaryBonusLine", "{0}件套：{1}"),
						FText::AsNumber(Bonus.RequiredPieceCount),
						Bonus.Description));
				}
			}
		}
	}

	return Lines.IsEmpty()
		? NSLOCTEXT("CharacterPanel", "RelicSetSummaryEmpty", "暂无套装效果。")
		: FText::Join(FText::FromString(TEXT("\n")), Lines);
}

FText UVM_CharacterPanelScreen::GetRelicSlotDisplayName(EHoyoRelicSlot Slot) const
{
	switch (Slot)
	{
	case EHoyoRelicSlot::Head:
		return NSLOCTEXT("CharacterPanel", "RelicSlotHead", "Head");
	case EHoyoRelicSlot::Hands:
		return NSLOCTEXT("CharacterPanel", "RelicSlotHands", "Hands");
	case EHoyoRelicSlot::Body:
		return NSLOCTEXT("CharacterPanel", "RelicSlotBody", "Body");
	case EHoyoRelicSlot::Feet:
		return NSLOCTEXT("CharacterPanel", "RelicSlotFeet", "Feet");
	case EHoyoRelicSlot::PlanarSphere:
		return NSLOCTEXT("CharacterPanel", "RelicSlotPlanarSphere", "Planar Sphere");
	case EHoyoRelicSlot::LinkRope:
		return NSLOCTEXT("CharacterPanel", "RelicSlotLinkRope", "Link Rope");
	default:
		return FText::GetEmpty();
	}
}
