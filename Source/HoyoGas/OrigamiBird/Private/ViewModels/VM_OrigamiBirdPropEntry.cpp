#include "ViewModels/VM_OrigamiBirdPropEntry.h"

#include "Engine/Texture2D.h"

void UVM_OrigamiBirdPropEntry::InitializeFromDefinition(FName InPropId, const FOrigamiBirdPropDefinitionRow& Definition, int32 InCount)
{
	SetPropId(InPropId);
	SetDisplayNameText(Definition.DisplayName.IsEmpty() ? FText::FromName(InPropId) : Definition.DisplayName);
	SetDescriptionText(Definition.Description);
	SetIsStackable(Definition.bStackable);
	SetCount(InCount);
	SetCountText(FText::Format(NSLOCTEXT("OrigamiBird", "PropCountFormat", "x{0}"), FText::AsNumber(InCount)));
	SetStackRuleText(Definition.bStackable
		? FText::Format(NSLOCTEXT("OrigamiBird", "PropStackableFormat", "Stack {0}"), FText::AsNumber(FMath::Max(1, Definition.MaxStackCount)))
		: NSLOCTEXT("OrigamiBird", "PropNonStackable", "Unique"));
	SetIconTexture(Definition.Icon.LoadSynchronous());
}

FName UVM_OrigamiBirdPropEntry::GetPropId() const
{
	return PropId;
}

void UVM_OrigamiBirdPropEntry::SetPropId(FName InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(PropId, InValue);
}

FText UVM_OrigamiBirdPropEntry::GetDisplayNameText() const
{
	return DisplayNameText;
}

void UVM_OrigamiBirdPropEntry::SetDisplayNameText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(DisplayNameText, InValue);
}

FText UVM_OrigamiBirdPropEntry::GetDescriptionText() const
{
	return DescriptionText;
}

void UVM_OrigamiBirdPropEntry::SetDescriptionText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(DescriptionText, InValue);
}

FText UVM_OrigamiBirdPropEntry::GetCountText() const
{
	return CountText;
}

void UVM_OrigamiBirdPropEntry::SetCountText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CountText, InValue);
}

FText UVM_OrigamiBirdPropEntry::GetStackRuleText() const
{
	return StackRuleText;
}

void UVM_OrigamiBirdPropEntry::SetStackRuleText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(StackRuleText, InValue);
}

bool UVM_OrigamiBirdPropEntry::GetIsStackable() const
{
	return bIsStackable;
}

void UVM_OrigamiBirdPropEntry::SetIsStackable(bool bInValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsStackable, bInValue);
}

int32 UVM_OrigamiBirdPropEntry::GetCount() const
{
	return Count;
}

void UVM_OrigamiBirdPropEntry::SetCount(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(Count, InValue);
}

UTexture2D* UVM_OrigamiBirdPropEntry::GetIconTexture() const
{
	return IconTexture;
}

void UVM_OrigamiBirdPropEntry::SetIconTexture(UTexture2D* InTexture)
{
	if (IconTexture == InTexture)
	{
		return;
	}

	IconTexture = InTexture;
}
