#include "ViewModels/VM_CharacterRelicSetBonusEntry.h"

FName UVM_CharacterRelicSetBonusEntry::GetSetId() const
{
	return SetId;
}

void UVM_CharacterRelicSetBonusEntry::SetSetId(FName InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SetId, InValue);
}

FText UVM_CharacterRelicSetBonusEntry::GetSetNameText() const
{
	return SetNameText;
}

void UVM_CharacterRelicSetBonusEntry::SetSetNameText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(SetNameText, InValue);
}

FText UVM_CharacterRelicSetBonusEntry::GetActivationText() const
{
	return ActivationText;
}

void UVM_CharacterRelicSetBonusEntry::SetActivationText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ActivationText, InValue);
}
