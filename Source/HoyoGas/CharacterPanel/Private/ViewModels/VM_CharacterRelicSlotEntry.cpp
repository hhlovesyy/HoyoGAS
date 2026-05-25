#include "ViewModels/VM_CharacterRelicSlotEntry.h"

EHoyoRelicSlot UVM_CharacterRelicSlotEntry::GetSlot() const
{
	return Slot;
}

void UVM_CharacterRelicSlotEntry::SetSlot(EHoyoRelicSlot InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(Slot, InValue);
}

FText UVM_CharacterRelicSlotEntry::GetSlotNameText() const
{
	return SlotNameText;
}

void UVM_CharacterRelicSlotEntry::SetSlotNameText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SlotNameText, InValue);
}

FText UVM_CharacterRelicSlotEntry::GetRelicNameText() const
{
	return RelicNameText;
}

void UVM_CharacterRelicSlotEntry::SetRelicNameText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(RelicNameText, InValue);
}

FText UVM_CharacterRelicSlotEntry::GetSetNameText() const
{
	return SetNameText;
}

void UVM_CharacterRelicSlotEntry::SetSetNameText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SetNameText, InValue);
}

FText UVM_CharacterRelicSlotEntry::GetLevelText() const
{
	return LevelText;
}

void UVM_CharacterRelicSlotEntry::SetLevelText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(LevelText, InValue);
}

FText UVM_CharacterRelicSlotEntry::GetMainAffixText() const
{
	return MainAffixText;
}

void UVM_CharacterRelicSlotEntry::SetMainAffixText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(MainAffixText, InValue);
}

FText UVM_CharacterRelicSlotEntry::GetSubAffixSummaryText() const
{
	return SubAffixSummaryText;
}

void UVM_CharacterRelicSlotEntry::SetSubAffixSummaryText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SubAffixSummaryText, InValue);
}

UTexture2D* UVM_CharacterRelicSlotEntry::GetIconTexture() const
{
	return IconTexture;
}

void UVM_CharacterRelicSlotEntry::SetIconTexture(UTexture2D* InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(IconTexture, InValue);
}

bool UVM_CharacterRelicSlotEntry::GetIsEquipped() const
{
	return bIsEquipped;
}

void UVM_CharacterRelicSlotEntry::SetIsEquipped(bool bInValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsEquipped, bInValue);
}
