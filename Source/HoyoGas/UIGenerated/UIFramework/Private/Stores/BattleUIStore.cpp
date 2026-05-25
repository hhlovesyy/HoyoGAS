#include "Stores/BattleUIStore.h"

#include "Battle/HoyoBattleEncounterDefinition.h"
#include "Character/HoyoEnemyInterface.h"
#include "Engine/LocalPlayer.h"
#include "Subsystems/HoyoBattleFlowSubsystem.h"

void UBattleUIStore::BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState)
{
	Super::BindToPlayerContext(InPawn, InPlayerState);
	BindBattleSources();
	BroadcastStoreChanged();
}

void UBattleUIStore::UnbindFromPlayerContext()
{
	UnbindBattleSources();
	Super::UnbindFromPlayerContext();
	BroadcastStoreChanged();
}

void UBattleUIStore::BindBattleSources()
{
	ULocalPlayer* StoreLocalPlayer = GetOwningLocalPlayer();
	UHoyoBattleFlowSubsystem* NewBattleFlowSubsystem = StoreLocalPlayer ? StoreLocalPlayer->GetSubsystem<UHoyoBattleFlowSubsystem>() : nullptr;
	if (BattleFlowSubsystem.Get() == NewBattleFlowSubsystem)
	{
		RefreshFromBattleFlow();
		return;
	}

	UnbindBattleSources();
	BattleFlowSubsystem = NewBattleFlowSubsystem;
	if (!BattleFlowSubsystem.IsValid())
	{
		ResetPresentationState();
		return;
	}

	BattleFlowSubsystem->OnFlowStateChanged().AddUObject(this, &UBattleUIStore::HandleFlowStateChanged);
	BattleFlowSubsystem->OnEncounterChanged().AddUObject(this, &UBattleUIStore::HandleEncounterChanged);
	BattleFlowSubsystem->OnHoveredEnemyChanged().AddUObject(this, &UBattleUIStore::HandleHoveredEnemyChanged);
	BattleFlowSubsystem->OnSelectedEnemyChanged().AddUObject(this, &UBattleUIStore::HandleSelectedEnemyChanged);
	RefreshFromBattleFlow();
}

void UBattleUIStore::UnbindBattleSources()
{
	if (BattleFlowSubsystem.IsValid())
	{
		BattleFlowSubsystem->OnFlowStateChanged().RemoveAll(this);
		BattleFlowSubsystem->OnEncounterChanged().RemoveAll(this);
		BattleFlowSubsystem->OnHoveredEnemyChanged().RemoveAll(this);
		BattleFlowSubsystem->OnSelectedEnemyChanged().RemoveAll(this);
	}

	BattleFlowSubsystem.Reset();
	HoveredEnemyActor.Reset();
	SelectedEnemyActor.Reset();
	ResetPresentationState();
}

const FText& UBattleUIStore::GetEncounterTitleText() const
{
	return EncounterTitleText;
}

const FText& UBattleUIStore::GetTargetStateText() const
{
	return TargetStateText;
}

const FText& UBattleUIStore::GetTargetNameText() const
{
	return TargetNameText;
}

const FText& UBattleUIStore::GetTargetLevelText() const
{
	return TargetLevelText;
}

const FText& UBattleUIStore::GetTargetAffinityText() const
{
	return TargetAffinityText;
}

const FText& UBattleUIStore::GetTargetHintText() const
{
	return TargetHintText;
}

const FText& UBattleUIStore::GetCurrentActorText() const
{
	return CurrentActorText;
}

const FText& UBattleUIStore::GetCommandHintText() const
{
	return CommandHintText;
}

const FText& UBattleUIStore::GetReticleDebugText() const
{
	return ReticleDebugText;
}

AActor* UBattleUIStore::GetHoveredEnemyActor() const
{
	return HoveredEnemyActor.Get();
}

AActor* UBattleUIStore::GetSelectedEnemyActor() const
{
	return SelectedEnemyActor.Get();
}

void UBattleUIStore::RefreshFromBattleFlow()
{
	const UHoyoBattleFlowSubsystem* FlowSubsystem = BattleFlowSubsystem.Get();
	if (!FlowSubsystem)
	{
		ResetPresentationState();
		BroadcastStoreChanged();
		return;
	}

	HoveredEnemyActor = FlowSubsystem->GetHoveredEnemy();
	SelectedEnemyActor = FlowSubsystem->GetSelectedEnemy();
	EncounterTitleText = BuildEncounterTitleText(FlowSubsystem->GetCurrentEncounter());
	CurrentActorText = FlowSubsystem->GetFlowState() == EHoyoBattleFlowState::TargetSelection
		? INVTEXT("Player Turn")
		: INVTEXT("Exploration");

	AActor* PrimaryTargetActor = ResolvePrimaryTargetActor();
	if (PrimaryTargetActor && PrimaryTargetActor->GetClass()->ImplementsInterface(UHoyoEnemyInterface::StaticClass()))
	{
		TargetNameText = IHoyoEnemyInterface::Execute_GetEnemyDisplayName(PrimaryTargetActor);
		TargetLevelText = FText::Format(INVTEXT("LV {0}"), FText::AsNumber(IHoyoEnemyInterface::Execute_GetEnemyLevel(PrimaryTargetActor)));
		TargetAffinityText = INVTEXT("Unknown");
		const bool bHasSelectedTarget = SelectedEnemyActor.Get() == PrimaryTargetActor;
		const bool bHasHoveredTarget = HoveredEnemyActor.Get() == PrimaryTargetActor;
		TargetStateText = bHasSelectedTarget ? INVTEXT("SELECTED") : (bHasHoveredTarget ? INVTEXT("HOVER") : INVTEXT("TARGET"));
		TargetHintText = bHasSelectedTarget
			? INVTEXT("Target locked. Choose an action.")
			: INVTEXT("Hover an enemy and click to confirm.");
		CommandHintText = bHasSelectedTarget
			? INVTEXT("Choose an action for the selected target.")
			: INVTEXT("Hover an enemy to inspect it.");
		ReticleDebugText = bHasSelectedTarget ? INVTEXT("[TARGET]") : INVTEXT("[AIM]");
	}
	else
	{
		TargetStateText = INVTEXT("NO TARGET");
		TargetNameText = INVTEXT("No Target");
		TargetLevelText = FText::GetEmpty();
		TargetAffinityText = FText::GetEmpty();
		TargetHintText = INVTEXT("Hover an enemy and click to confirm.");
		CommandHintText = INVTEXT("Choose a target.");
		ReticleDebugText = FText::GetEmpty();
	}

	BroadcastStoreChanged();
}

void UBattleUIStore::ResetPresentationState()
{
	EncounterTitleText = FText::GetEmpty();
	TargetStateText = INVTEXT("NO TARGET");
	TargetNameText = INVTEXT("No Target");
	TargetLevelText = FText::GetEmpty();
	TargetAffinityText = FText::GetEmpty();
	TargetHintText = INVTEXT("Hover an enemy and click to confirm.");
	CurrentActorText = INVTEXT("Player Turn");
	CommandHintText = INVTEXT("Choose a target.");
	ReticleDebugText = FText::GetEmpty();
}

FText UBattleUIStore::BuildEncounterTitleText(const UHoyoBattleEncounterDefinition* EncounterDefinition) const
{
	if (!EncounterDefinition)
	{
		return INVTEXT("Battle");
	}

	const FName EncounterId = EncounterDefinition->GetEncounterId();
	return EncounterId != NAME_None ? FText::FromName(EncounterId) : INVTEXT("Battle");
}

AActor* UBattleUIStore::ResolvePrimaryTargetActor() const
{
	if (AActor* SelectedTarget = SelectedEnemyActor.Get())
	{
		return SelectedTarget;
	}

	return HoveredEnemyActor.Get();
}

void UBattleUIStore::HandleFlowStateChanged(EHoyoBattleFlowState PreviousState, EHoyoBattleFlowState NewState)
{
	(void)PreviousState;
	(void)NewState;
	RefreshFromBattleFlow();
}

void UBattleUIStore::HandleEncounterChanged(const UHoyoBattleEncounterDefinition* EncounterDefinition)
{
	(void)EncounterDefinition;
	RefreshFromBattleFlow();
}

void UBattleUIStore::HandleHoveredEnemyChanged(AActor* PreviousEnemyActor, AActor* NewEnemyActor)
{
	(void)PreviousEnemyActor;
	(void)NewEnemyActor;
	RefreshFromBattleFlow();
}

void UBattleUIStore::HandleSelectedEnemyChanged(AActor* PreviousEnemyActor, AActor* NewEnemyActor)
{
	(void)PreviousEnemyActor;
	(void)NewEnemyActor;
	RefreshFromBattleFlow();
}
