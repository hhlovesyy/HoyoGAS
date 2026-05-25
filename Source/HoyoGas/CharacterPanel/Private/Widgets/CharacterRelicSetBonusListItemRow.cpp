#include "Widgets/CharacterRelicSetBonusListItemRow.h"

#include "INotifyFieldValueChanged.h"
#include "Styling/CoreStyle.h"
#include "ViewModels/VM_CharacterRelicSetBonusEntry.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UCharacterRelicSetBonusListItemRow::RebuildWidget()
{
	SAssignNew(RowBorder, SBorder)
		.Padding(FMargin(10.0f, 8.0f))
		.BorderBackgroundColor(FLinearColor(0.13f, 0.15f, 0.17f, 1.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SAssignNew(SetNameTextBlock, STextBlock)
				.Font(MakeFont(15))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.94f, 0.92f, 0.86f, 1.0f)))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SAssignNew(ActivationTextBlock, STextBlock)
				.Font(MakeFont(13))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.68f, 0.72f, 0.72f, 1.0f)))
			]
		];

	RefreshVisuals();
	return RowBorder.ToSharedRef();
}

void UCharacterRelicSetBonusListItemRow::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	BindViewModel(Cast<UVM_CharacterRelicSetBonusEntry>(ListItemObject));
}

void UCharacterRelicSetBonusListItemRow::NativeOnEntryReleased()
{
	IUserListEntry::NativeOnEntryReleased();
	BindViewModel(nullptr);
}

void UCharacterRelicSetBonusListItemRow::BindViewModel(UVM_CharacterRelicSetBonusEntry* InViewModel)
{
	if (ViewModel)
	{
		ViewModel->RemoveAllFieldValueChangedDelegates(this);
	}

	ViewModel = InViewModel;
	if (ViewModel)
	{
		const INotifyFieldValueChanged::FFieldValueChangedDelegate Delegate =
			INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateUObject(this, &UCharacterRelicSetBonusListItemRow::HandleViewModelFieldChanged);
		ViewModel->AddFieldValueChangedDelegate(UVM_CharacterRelicSetBonusEntry::FFieldNotificationClassDescriptor::SetNameText, Delegate);
		ViewModel->AddFieldValueChangedDelegate(UVM_CharacterRelicSetBonusEntry::FFieldNotificationClassDescriptor::ActivationText, Delegate);
	}

	RefreshVisuals();
}

void UCharacterRelicSetBonusListItemRow::RefreshVisuals()
{
	if (SetNameTextBlock)
	{
		SetNameTextBlock->SetText(ViewModel ? ViewModel->GetSetNameText() : FText::GetEmpty());
	}
	if (ActivationTextBlock)
	{
		ActivationTextBlock->SetText(ViewModel ? ViewModel->GetActivationText() : FText::GetEmpty());
	}
}

void UCharacterRelicSetBonusListItemRow::HandleViewModelFieldChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId)
{
	(void)Object;
	(void)FieldId;
	RefreshVisuals();
}

FSlateFontInfo UCharacterRelicSetBonusListItemRow::MakeFont(int32 Size) const
{
	FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle("Regular", Size);
	if (UObject* FontObject = LoadObject<UObject>(nullptr, TEXT("/Game/Genshin/Fonts/Genshin_Font.Genshin_Font")))
	{
		FontInfo.FontObject = FontObject;
	}
	return FontInfo;
}
