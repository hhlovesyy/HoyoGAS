#include "Subsystems/BattleScreenFlowRule.h"

#include "Battle/HoyoBattleEncounterDefinition.h"
#include "Engine/LocalPlayer.h"
#include "GameplayDemoUIConstants.h"
#include "Subsystems/HoyoBattleFlowSubsystem.h"
#include "Subsystems/MyPlayerUISubsystem.h"
#include "Widgets/MyScreenBase.h"

void UBattleScreenFlowRule::Deinitialize()
{
	CloseBattleScreenIfNeeded();
	UnbindBattleFlow();
	Super::Deinitialize();
}

void UBattleScreenFlowRule::RefreshBindings()
{
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	UHoyoBattleFlowSubsystem* NewBattleFlowSubsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UHoyoBattleFlowSubsystem>() : nullptr;

	if (BattleFlowSubsystem.Get() == NewBattleFlowSubsystem)
	{
		EvaluateBattleScreenFlow();
		return;
	}

	UnbindBattleFlow();
	BattleFlowSubsystem = NewBattleFlowSubsystem;

	if (BattleFlowSubsystem.IsValid())
	{
		BattleFlowSubsystem->OnFlowStateChanged().AddUObject(this, &UBattleScreenFlowRule::HandleBattleFlowStateChanged);
		BattleFlowSubsystem->OnEncounterChanged().AddUObject(this, &UBattleScreenFlowRule::HandleBattleEncounterChanged);
	}

	EvaluateBattleScreenFlow();
}

void UBattleScreenFlowRule::HandleBattleFlowStateChanged(EHoyoBattleFlowState PreviousState, EHoyoBattleFlowState NewState)
{
	(void)PreviousState;
	(void)NewState;
	EvaluateBattleScreenFlow();
}

void UBattleScreenFlowRule::HandleBattleEncounterChanged(const UHoyoBattleEncounterDefinition* EncounterDefinition)
{
	(void)EncounterDefinition;
	EvaluateBattleScreenFlow();
}

void UBattleScreenFlowRule::EvaluateBattleScreenFlow()
{
	if (!BattleFlowSubsystem.IsValid())
	{
		CloseBattleScreenIfNeeded();
		return;
	}

	switch (BattleFlowSubsystem->GetFlowState())
	{
	case EHoyoBattleFlowState::TargetSelection:
		OpenBattleScreenIfNeeded();
		break;
	case EHoyoBattleFlowState::Exploration:
	case EHoyoBattleFlowState::ExitingBattle:
		CloseBattleScreenIfNeeded();
		break;
	case EHoyoBattleFlowState::EnteringBattle:
	default:
		break;
	}
}

void UBattleScreenFlowRule::OpenBattleScreenIfNeeded()
{
	UMyPlayerUISubsystem* PlayerUISubsystem = GetPlayerUISubsystem();
	if (!PlayerUISubsystem || !BattleFlowSubsystem.IsValid())
	{
		return;
	}

	const UHoyoBattleEncounterDefinition* EncounterDefinition = BattleFlowSubsystem->GetCurrentEncounter();
	const FName RequestedScreenTag = EncounterDefinition && !EncounterDefinition->GetBattleScreenTag().IsNone()
		? EncounterDefinition->GetBattleScreenTag()
		: FGameplayDemoUIConstants::BattleScreenTag();

	if (RequestedScreenTag.IsNone())
	{
		return;
	}

	if (bBattleScreenRequested && ActiveBattleScreenTag == RequestedScreenTag && PlayerUISubsystem->FindScreenByTag(RequestedScreenTag))
	{
		HideExplorationHUDIfNeeded();
		return;
	}

	if (bBattleScreenRequested && ActiveBattleScreenTag != RequestedScreenTag)
	{
		CloseBattleScreenIfNeeded();
	}

	if (PlayerUISubsystem->OpenScreen(RequestedScreenTag, FMyUIPayload()))
	{
		ActiveBattleScreenTag = RequestedScreenTag;
		bBattleScreenRequested = true;
		HideExplorationHUDIfNeeded();
	}
}

void UBattleScreenFlowRule::CloseBattleScreenIfNeeded()
{
	if (!bBattleScreenRequested || ActiveBattleScreenTag.IsNone())
	{
		bBattleScreenRequested = false;
		ActiveBattleScreenTag = NAME_None;
		return;
	}

	if (UMyPlayerUISubsystem* PlayerUISubsystem = GetPlayerUISubsystem())
	{
		if (UMyScreenBase* BattleScreen = PlayerUISubsystem->FindScreenByTag(ActiveBattleScreenTag))
		{
			PlayerUISubsystem->CloseScreen(BattleScreen);
		}
	}

	bBattleScreenRequested = false;
	ActiveBattleScreenTag = NAME_None;
	RestoreExplorationHUDIfNeeded();
}

void UBattleScreenFlowRule::HideExplorationHUDIfNeeded()
{
	if (bExplorationHUDHiddenForBattle)
	{
		return;
	}

	if (UMyPlayerUISubsystem* PlayerUISubsystem = GetPlayerUISubsystem())
	{
		if (UMyScreenBase* HUDScreen = PlayerUISubsystem->FindScreenByTag(FGameplayDemoUIConstants::HUDScreenTag()))
		{
			PlayerUISubsystem->CloseScreen(HUDScreen);
			bExplorationHUDHiddenForBattle = true;
		}
	}
}

void UBattleScreenFlowRule::RestoreExplorationHUDIfNeeded()
{
	if (!bExplorationHUDHiddenForBattle)
	{
		return;
	}

	if (UMyPlayerUISubsystem* PlayerUISubsystem = GetPlayerUISubsystem())
	{
		if (!PlayerUISubsystem->FindScreenByTag(FGameplayDemoUIConstants::HUDScreenTag()))
		{
			PlayerUISubsystem->OpenScreen(FGameplayDemoUIConstants::HUDScreenTag(), FMyUIPayload());
		}
	}

	bExplorationHUDHiddenForBattle = false;
}

void UBattleScreenFlowRule::UnbindBattleFlow()
{
	if (BattleFlowSubsystem.IsValid())
	{
		BattleFlowSubsystem->OnFlowStateChanged().RemoveAll(this);
		BattleFlowSubsystem->OnEncounterChanged().RemoveAll(this);
	}

	BattleFlowSubsystem.Reset();
}
