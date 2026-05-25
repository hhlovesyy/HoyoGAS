#include "ViewModels/VM_CharacterStoryEntry.h"

FName UVM_CharacterStoryEntry::GetStoryId() const
{
	return StoryId;
}

void UVM_CharacterStoryEntry::SetStoryId(FName InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(StoryId, InValue);
}

int32 UVM_CharacterStoryEntry::GetStoryIndex() const
{
	return StoryIndex;
}

void UVM_CharacterStoryEntry::SetStoryIndex(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(StoryIndex, InValue);
}

FText UVM_CharacterStoryEntry::GetTitleText() const
{
	return TitleText;
}

void UVM_CharacterStoryEntry::SetTitleText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(TitleText, InValue);
}

bool UVM_CharacterStoryEntry::GetIsUnlocked() const
{
	return bIsUnlocked;
}

void UVM_CharacterStoryEntry::SetIsUnlocked(bool bInValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsUnlocked, bInValue);
}

bool UVM_CharacterStoryEntry::GetIsSelected() const
{
	return bIsSelected;
}

void UVM_CharacterStoryEntry::SetIsSelected(bool bInValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsSelected, bInValue);
}
