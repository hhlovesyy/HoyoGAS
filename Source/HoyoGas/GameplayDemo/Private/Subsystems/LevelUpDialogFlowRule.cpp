#include "Subsystems/LevelUpDialogFlowRule.h"

#include "Engine/LocalPlayer.h"
#include "GameplayDemoUIConstants.h"
#include "Stores/MyUIStoreTypes.h"
#include "Stores/ProgressionUIStore.h"
#include "Subsystems/MyPlayerUISubsystem.h"
#include "Subsystems/MyUIStoreSubsystem.h"

void ULevelUpDialogFlowRule::Deinitialize()
{
	UnbindProgressionStore();
	Super::Deinitialize();
}

void ULevelUpDialogFlowRule::RefreshBindings()
{
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	UProgressionUIStore* NewProgressionStore = nullptr;

	if (UMyUIStoreSubsystem* StoreSubsystem = LocalPlayer ? ULocalPlayer::GetSubsystem<UMyUIStoreSubsystem>(LocalPlayer) : nullptr)
	{
		StoreSubsystem->RefreshPlayerContext();
		NewProgressionStore = StoreSubsystem->GetStore<UProgressionUIStore>();
	}

	if (ProgressionStore.Get() != NewProgressionStore)
	{
		UnbindProgressionStore();

		if (NewProgressionStore)
		{
			ProgressionStore = NewProgressionStore;
			ProgressionStore->OnStoreChanged().AddUObject(this, &ULevelUpDialogFlowRule::HandleStoreChanged);
		}
	}

	EvaluateLevelUpFlow();
}

void ULevelUpDialogFlowRule::HandleStoreChanged(UUIStoreBase* ChangedStore)
{
	EvaluateLevelUpFlow();
}

void ULevelUpDialogFlowRule::EvaluateLevelUpFlow()
{
	if (!ProgressionStore.IsValid())
	{
		return;
	}

	if (!ProgressionStore->HasPendingLevelUp())
	{
		bLevelUpDialogRequested = false;
		return;
	}

	UMyPlayerUISubsystem* PlayerUISubsystem = GetPlayerUISubsystem();
	if (!PlayerUISubsystem)
	{
		return;
	}

	if (PlayerUISubsystem->FindScreenByTag(FGameplayDemoUIConstants::LevelUpDialogTag()))
	{
		bLevelUpDialogRequested = true;
		return;
	}

	if (bLevelUpDialogRequested)
	{
		return;
	}

	if (PlayerUISubsystem->ShowDialog(FGameplayDemoUIConstants::LevelUpDialogTag(), FMyUIPayload()))
	{
		bLevelUpDialogRequested = true;
	}
}

void ULevelUpDialogFlowRule::UnbindProgressionStore()
{
	if (ProgressionStore.IsValid())
	{
		ProgressionStore->OnStoreChanged().RemoveAll(this);
		ProgressionStore.Reset();
	}

	bLevelUpDialogRequested = false;
}
