#include "Widgets/CharacterRelicSlotListItemRow.h"

#include "INotifyFieldValueChanged.h"
#include "Styling/CoreStyle.h"
#include "ViewModels/VM_CharacterRelicSlotEntry.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UCharacterRelicSlotListItemRow::RebuildWidget()
{
	SAssignNew(RowBorder, SBorder)
		.Padding(FMargin(12.0f, 10.0f))
		.BorderBackgroundColor(FLinearColor(0.13f, 0.15f, 0.17f, 1.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SAssignNew(SlotTextBlock, STextBlock)
				.Font(MakeFont(15))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.68f, 0.72f, 0.72f, 1.0f)))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SAssignNew(RelicNameTextBlock, STextBlock)
				.Font(MakeFont(18))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.94f, 0.92f, 0.86f, 1.0f)))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f, 0.0f, 0.0f)
			[
				SAssignNew(SetNameTextBlock, STextBlock)
				.Font(MakeFont(14))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.80f, 0.78f, 1.0f)))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f, 0.0f, 0.0f)
			[
				SAssignNew(LevelTextBlock, STextBlock)
				.Font(MakeFont(14))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.78f, 0.42f, 1.0f)))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SAssignNew(MainAffixTextBlock, STextBlock)
				.Font(MakeFont(15))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.88f, 0.90f, 0.90f, 1.0f)))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f, 0.0f, 0.0f)
			[
				SAssignNew(SubAffixTextBlock, STextBlock)
				.Font(MakeFont(13))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.68f, 0.72f, 0.72f, 1.0f)))
			]
		];

	RefreshVisuals();
	return RowBorder.ToSharedRef();
}

void UCharacterRelicSlotListItemRow::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	BindViewModel(Cast<UVM_CharacterRelicSlotEntry>(ListItemObject));
}

void UCharacterRelicSlotListItemRow::NativeOnEntryReleased()
{
	IUserListEntry::NativeOnEntryReleased();
	BindViewModel(nullptr);
}

void UCharacterRelicSlotListItemRow::BindViewModel(UVM_CharacterRelicSlotEntry* InViewModel)
{
	if (ViewModel)
	{
		ViewModel->RemoveAllFieldValueChangedDelegates(this);
	}

	ViewModel = InViewModel;
	if (ViewModel)
	{
		const INotifyFieldValueChanged::FFieldValueChangedDelegate Delegate =
			INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateUObject(this, &UCharacterRelicSlotListItemRow::HandleViewModelFieldChanged);
		ViewModel->AddFieldValueChangedDelegate(UVM_CharacterRelicSlotEntry::FFieldNotificationClassDescriptor::SlotNameText, Delegate);
		ViewModel->AddFieldValueChangedDelegate(UVM_CharacterRelicSlotEntry::FFieldNotificationClassDescriptor::RelicNameText, Delegate);
		ViewModel->AddFieldValueChangedDelegate(UVM_CharacterRelicSlotEntry::FFieldNotificationClassDescriptor::SetNameText, Delegate);
		ViewModel->AddFieldValueChangedDelegate(UVM_CharacterRelicSlotEntry::FFieldNotificationClassDescriptor::LevelText, Delegate);
		ViewModel->AddFieldValueChangedDelegate(UVM_CharacterRelicSlotEntry::FFieldNotificationClassDescriptor::MainAffixText, Delegate);
		ViewModel->AddFieldValueChangedDelegate(UVM_CharacterRelicSlotEntry::FFieldNotificationClassDescriptor::SubAffixSummaryText, Delegate);
		ViewModel->AddFieldValueChangedDelegate(UVM_CharacterRelicSlotEntry::FFieldNotificationClassDescriptor::bIsEquipped, Delegate);
	}

	RefreshVisuals();
}

void UCharacterRelicSlotListItemRow::RefreshVisuals()
{
	const bool bEquipped = ViewModel && ViewModel->GetIsEquipped();
	if (RowBorder)
	{
		RowBorder->SetBorderBackgroundColor(bEquipped
			? FLinearColor(0.15f, 0.18f, 0.20f, 1.0f)
			: FLinearColor(0.10f, 0.11f, 0.12f, 1.0f));
	}

	if (SlotTextBlock)
	{
		SlotTextBlock->SetText(ViewModel ? ViewModel->GetSlotNameText() : FText::GetEmpty());
	}
	if (RelicNameTextBlock)
	{
		RelicNameTextBlock->SetText(ViewModel ? ViewModel->GetRelicNameText() : FText::GetEmpty());
	}
	if (SetNameTextBlock)
	{
		SetNameTextBlock->SetText(ViewModel ? ViewModel->GetSetNameText() : FText::GetEmpty());
	}
	if (LevelTextBlock)
	{
		LevelTextBlock->SetText(ViewModel ? ViewModel->GetLevelText() : FText::GetEmpty());
	}
	if (MainAffixTextBlock)
	{
		MainAffixTextBlock->SetText(ViewModel ? ViewModel->GetMainAffixText() : FText::GetEmpty());
	}
	if (SubAffixTextBlock)
	{
		SubAffixTextBlock->SetText(ViewModel ? ViewModel->GetSubAffixSummaryText() : FText::GetEmpty());
	}
}

void UCharacterRelicSlotListItemRow::HandleViewModelFieldChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId)
{
	(void)Object;
	(void)FieldId;
	RefreshVisuals();
}

FSlateFontInfo UCharacterRelicSlotListItemRow::MakeFont(int32 Size) const
{
	FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle("Regular", Size);
	if (UObject* FontObject = LoadObject<UObject>(nullptr, TEXT("/Game/Genshin/Fonts/Genshin_Font.Genshin_Font")))
	{
		FontInfo.FontObject = FontObject;
	}
	return FontInfo;
}
