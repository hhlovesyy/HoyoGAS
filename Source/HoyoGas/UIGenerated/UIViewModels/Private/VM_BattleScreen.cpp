#include "VM_BattleScreen.h"

#include "Stores/BattleUIStore.h"
#include "Stores/MyUIStoreBase.h"

void UVM_BattleScreen::Initialize(UBattleUIStore* InBattleStore)
{
	if (BattleStore.Get() != InBattleStore)
	{
		if (BattleStore.IsValid())
		{
			BattleStore->OnStoreChanged().RemoveAll(this);
		}

		BattleStore = InBattleStore;
		if (BattleStore.IsValid())
		{
			BattleStore->OnStoreChanged().AddUObject(this, &UVM_BattleScreen::HandleStoreChanged);
		}
	}

	RefreshFromStore();
}

void UVM_BattleScreen::Teardown()
{
	if (BattleStore.IsValid())
	{
		BattleStore->OnStoreChanged().RemoveAll(this);
		BattleStore.Reset();
	}
}

FText UVM_BattleScreen::GetCommandHintText() const
{
	return CommandHintText;
}

void UVM_BattleScreen::SetCommandHintText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CommandHintText, InValue);
}

FText UVM_BattleScreen::GetCurrentActorText() const
{
	return CurrentActorText;
}

void UVM_BattleScreen::SetCurrentActorText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentActorText, InValue);
}

FText UVM_BattleScreen::GetEncounterTitleText() const
{
	return EncounterTitleText;
}

void UVM_BattleScreen::SetEncounterTitleText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(EncounterTitleText, InValue);
}

FText UVM_BattleScreen::GetReticleDebugText() const
{
	return ReticleDebugText;
}

void UVM_BattleScreen::SetReticleDebugText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ReticleDebugText, InValue);
}

FText UVM_BattleScreen::GetTargetHintText() const
{
	return TargetHintText;
}

void UVM_BattleScreen::SetTargetHintText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(TargetHintText, InValue);
}

FText UVM_BattleScreen::GetTargetAffinityText() const
{
	return TargetAffinityText;
}

void UVM_BattleScreen::SetTargetAffinityText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(TargetAffinityText, InValue);
}

FText UVM_BattleScreen::GetTargetLevelText() const
{
	return TargetLevelText;
}

void UVM_BattleScreen::SetTargetLevelText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(TargetLevelText, InValue);
}

FText UVM_BattleScreen::GetTargetNameText() const
{
	return TargetNameText;
}

void UVM_BattleScreen::SetTargetNameText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(TargetNameText, InValue);
}

FText UVM_BattleScreen::GetTargetStateText() const
{
	return TargetStateText;
}

void UVM_BattleScreen::SetTargetStateText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(TargetStateText, InValue);
}

void UVM_BattleScreen::RefreshFromStore()
{
	SetCommandHintText(BattleStore.IsValid() ? BattleStore->GetCommandHintText() : FText::GetEmpty());
	SetCurrentActorText(BattleStore.IsValid() ? BattleStore->GetCurrentActorText() : FText::GetEmpty());
	SetEncounterTitleText(BattleStore.IsValid() ? BattleStore->GetEncounterTitleText() : FText::GetEmpty());
	SetReticleDebugText(BattleStore.IsValid() ? BattleStore->GetReticleDebugText() : FText::GetEmpty());
	SetTargetHintText(BattleStore.IsValid() ? BattleStore->GetTargetHintText() : FText::GetEmpty());
	SetTargetAffinityText(BattleStore.IsValid() ? BattleStore->GetTargetAffinityText() : FText::GetEmpty());
	SetTargetLevelText(BattleStore.IsValid() ? BattleStore->GetTargetLevelText() : FText::GetEmpty());
	SetTargetNameText(BattleStore.IsValid() ? BattleStore->GetTargetNameText() : FText::GetEmpty());
	SetTargetStateText(BattleStore.IsValid() ? BattleStore->GetTargetStateText() : FText::GetEmpty());
}

void UVM_BattleScreen::HandleStoreChanged(UUIStoreBase* ChangedStore)
{
	(void)ChangedStore;
	RefreshFromStore();
}
