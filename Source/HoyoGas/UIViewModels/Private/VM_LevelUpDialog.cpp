#include "VM_LevelUpDialog.h"

#include "Stores/ProgressionUIStore.h"

void UVM_LevelUpDialog::Initialize(UProgressionUIStore* InProgressionStore)
{
	if (!InProgressionStore)
	{
		SetNewLevelText(INVTEXT("Level Up"));
		SetRewardText(INVTEXT("Progress milestone reached."));
		return;
	}

	const FPendingLevelUpSnapshot Snapshot = InProgressionStore->GetPendingLevelUpSnapshot();
	SetNewLevelText(Snapshot.PendingLevelText.IsEmpty() ? INVTEXT("Level Up") : Snapshot.PendingLevelText);
	SetRewardText(Snapshot.PendingRewardText.IsEmpty() ? INVTEXT("Progress milestone reached.") : Snapshot.PendingRewardText);
	InProgressionStore->ConsumePendingLevelUp();
}

FText UVM_LevelUpDialog::GetNewLevelText() const
{
	return NewLevelText;
}

void UVM_LevelUpDialog::SetNewLevelText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(NewLevelText, InValue);
}

FText UVM_LevelUpDialog::GetRewardText() const
{
	return RewardText;
}

void UVM_LevelUpDialog::SetRewardText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(RewardText, InValue);
}
