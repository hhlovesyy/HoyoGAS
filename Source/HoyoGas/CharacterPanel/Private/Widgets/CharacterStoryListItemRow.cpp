#include "Widgets/CharacterStoryListItemRow.h"

#include "INotifyFieldValueChanged.h"
#include "ViewModels/VM_CharacterStoryEntry.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UCharacterStoryListItemRow::RebuildWidget()
{
	SAssignNew(RowBorder, SBorder)
		.Padding(FMargin(12.0f, 8.0f))
		.BorderBackgroundColor(FLinearColor(0.16f, 0.18f, 0.20f, 1.0f))
		[
			SAssignNew(TitleTextBlock, STextBlock)
			.Font(MakeTitleFont())
			.ColorAndOpacity(FSlateColor(FLinearColor(0.86f, 0.88f, 0.88f, 1.0f)))
			.Text(FText::GetEmpty())
		];

	RefreshVisuals();
	return RowBorder.ToSharedRef();
}

void UCharacterStoryListItemRow::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	BindViewModel(Cast<UVM_CharacterStoryEntry>(ListItemObject));
}

void UCharacterStoryListItemRow::NativeOnItemSelectionChanged(bool bIsSelected)
{
	IUserListEntry::NativeOnItemSelectionChanged(bIsSelected);
	if (ViewModel && ViewModel->GetIsSelected() != bIsSelected)
	{
		ViewModel->SetIsSelected(bIsSelected);
	}
}

void UCharacterStoryListItemRow::NativeOnEntryReleased()
{
	IUserListEntry::NativeOnEntryReleased();
	BindViewModel(nullptr);
}

void UCharacterStoryListItemRow::RefreshVisuals()
{
	if (RowBorder)
	{
		RowBorder->SetBorderBackgroundColor(ViewModel && ViewModel->GetIsSelected()
			? FLinearColor(0.24f, 0.28f, 0.31f, 1.0f)
			: FLinearColor(0.13f, 0.15f, 0.17f, 1.0f));
	}

	if (TitleTextBlock)
	{
		TitleTextBlock->SetText(ViewModel ? ViewModel->GetTitleText() : FText::GetEmpty());
		TitleTextBlock->SetColorAndOpacity(FSlateColor(ViewModel && ViewModel->GetIsUnlocked()
			? FLinearColor(0.92f, 0.92f, 0.88f, 1.0f)
			: FLinearColor(0.48f, 0.50f, 0.50f, 1.0f)));
	}
}

void UCharacterStoryListItemRow::BindViewModel(UVM_CharacterStoryEntry* InViewModel)
{
	if (ViewModel)
	{
		ViewModel->RemoveAllFieldValueChangedDelegates(this);
	}

	ViewModel = InViewModel;
	if (ViewModel)
	{
		const INotifyFieldValueChanged::FFieldValueChangedDelegate Delegate =
			INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateUObject(this, &UCharacterStoryListItemRow::HandleViewModelFieldChanged);
		ViewModel->AddFieldValueChangedDelegate(UVM_CharacterStoryEntry::FFieldNotificationClassDescriptor::TitleText, Delegate);
		ViewModel->AddFieldValueChangedDelegate(UVM_CharacterStoryEntry::FFieldNotificationClassDescriptor::bIsUnlocked, Delegate);
		ViewModel->AddFieldValueChangedDelegate(UVM_CharacterStoryEntry::FFieldNotificationClassDescriptor::bIsSelected, Delegate);
	}

	RefreshVisuals();
}

void UCharacterStoryListItemRow::HandleViewModelFieldChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId)
{
	(void)Object;
	(void)FieldId;
	RefreshVisuals();
}

FSlateFontInfo UCharacterStoryListItemRow::MakeTitleFont() const
{
	FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle("Regular", 16);
	if (UObject* FontObject = LoadObject<UObject>(nullptr, TEXT("/Game/Genshin/Fonts/Genshin_Font.Genshin_Font")))
	{
		FontInfo.FontObject = FontObject;
	}
	return FontInfo;
}
