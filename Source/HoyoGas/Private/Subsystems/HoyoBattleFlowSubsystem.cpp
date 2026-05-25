// Copyright Epic Games, Inc. All Rights Reserved.

#include "Subsystems/HoyoBattleFlowSubsystem.h"

#include "Battle/HoyoBattleEnemyActor.h"
#include "Battle/HoyoBattleEncounterDefinition.h"
#include "Character/HoyoEnemyInterface.h"
#include "Engine/World.h"

void UHoyoBattleFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetBattleSession();
}

void UHoyoBattleFlowSubsystem::Deinitialize()
{
	ResetBattleSession();
	Super::Deinitialize();
}

bool UHoyoBattleFlowSubsystem::RequestEnterBattle(UHoyoBattleEncounterDefinition* EncounterDefinition, const FTransform& EncounterOriginTransform)
{
	if (!EncounterDefinition)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHoyoBattleFlowSubsystem::RequestEnterBattle failed because EncounterDefinition is null."));
		return false;
	}

	if (FlowState != EHoyoBattleFlowState::Exploration)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHoyoBattleFlowSubsystem::RequestEnterBattle ignored because flow state is '%d'."), static_cast<int32>(FlowState));
		return false;
	}

	ResetBattleSession();
	CurrentEncounter = EncounterDefinition;
	CurrentEncounterOriginTransform = EncounterOriginTransform;
	EncounterChangedEvent.Broadcast(CurrentEncounter);
	SetFlowState(EHoyoBattleFlowState::EnteringBattle);
	return SpawnEncounterEnemies() && EnterTargetSelectionState();
}

bool UHoyoBattleFlowSubsystem::EnterTargetSelectionState()
{
	if (!CurrentEncounter || FlowState == EHoyoBattleFlowState::Exploration || FlowState == EHoyoBattleFlowState::ExitingBattle)
	{
		return false;
	}

	SetFlowState(EHoyoBattleFlowState::TargetSelection);
	return true;
}

bool UHoyoBattleFlowSubsystem::BeginBattleExit()
{
	if (FlowState == EHoyoBattleFlowState::Exploration || FlowState == EHoyoBattleFlowState::ExitingBattle)
	{
		return false;
	}

	SetFlowState(EHoyoBattleFlowState::ExitingBattle);
	return true;
}

void UHoyoBattleFlowSubsystem::CompleteBattleExit()
{
	ResetBattleSession();
}

EHoyoBattleFlowState UHoyoBattleFlowSubsystem::GetFlowState() const
{
	return FlowState;
}

bool UHoyoBattleFlowSubsystem::IsBattleActive() const
{
	return FlowState != EHoyoBattleFlowState::Exploration;
}

UHoyoBattleEncounterDefinition* UHoyoBattleFlowSubsystem::GetCurrentEncounter() const
{
	return CurrentEncounter;
}

FName UHoyoBattleFlowSubsystem::GetCurrentEncounterId() const
{
	return CurrentEncounter ? CurrentEncounter->GetEncounterId() : NAME_None;
}

FTransform UHoyoBattleFlowSubsystem::GetCurrentEncounterOriginTransform() const
{
	return CurrentEncounterOriginTransform;
}

void UHoyoBattleFlowSubsystem::RegisterBattleEnemy(AActor* EnemyActor)
{
	if (!EnemyActor)
	{
		return;
	}

	PruneInvalidBattleEnemies();
	if (ActiveBattleEnemies.Contains(EnemyActor))
	{
		return;
	}

	ActiveBattleEnemies.Add(EnemyActor);
}

void UHoyoBattleFlowSubsystem::UnregisterBattleEnemy(AActor* EnemyActor)
{
	if (!EnemyActor)
	{
		return;
	}

	ActiveBattleEnemies.Remove(EnemyActor);

	if (HoveredEnemyActor.Get() == EnemyActor)
	{
		SetHoveredEnemy(nullptr);
	}

	if (SelectedEnemyActor.Get() == EnemyActor)
	{
		SetSelectedEnemy(nullptr);
	}
}

void UHoyoBattleFlowSubsystem::GetActiveBattleEnemies(TArray<AActor*>& OutEnemies) const
{
	OutEnemies.Reset();

	for (const TWeakObjectPtr<AActor>& EnemyPtr : ActiveBattleEnemies)
	{
		if (AActor* EnemyActor = EnemyPtr.Get())
		{
			OutEnemies.Add(EnemyActor);
		}
	}
}

void UHoyoBattleFlowSubsystem::SetHoveredEnemy(AActor* EnemyActor)
{
	AActor* PreviousHoveredEnemy = HoveredEnemyActor.Get();
	if (PreviousHoveredEnemy == EnemyActor)
	{
		return;
	}

	HoveredEnemyActor = EnemyActor;
	HoveredEnemyChangedEvent.Broadcast(PreviousHoveredEnemy, HoveredEnemyActor.Get());
}

AActor* UHoyoBattleFlowSubsystem::GetHoveredEnemy() const
{
	return HoveredEnemyActor.Get();
}

void UHoyoBattleFlowSubsystem::SetSelectedEnemy(AActor* EnemyActor)
{
	AActor* PreviousSelectedEnemy = SelectedEnemyActor.Get();
	if (PreviousSelectedEnemy == EnemyActor)
	{
		return;
	}

	SelectedEnemyActor = EnemyActor;
	SelectedEnemyChangedEvent.Broadcast(PreviousSelectedEnemy, SelectedEnemyActor.Get());
}

AActor* UHoyoBattleFlowSubsystem::GetSelectedEnemy() const
{
	return SelectedEnemyActor.Get();
}

FOnHoyoBattleFlowStateChanged& UHoyoBattleFlowSubsystem::OnFlowStateChanged()
{
	return FlowStateChangedEvent;
}

FOnHoyoBattleEncounterChanged& UHoyoBattleFlowSubsystem::OnEncounterChanged()
{
	return EncounterChangedEvent;
}

FOnHoyoBattleHoveredEnemyChanged& UHoyoBattleFlowSubsystem::OnHoveredEnemyChanged()
{
	return HoveredEnemyChangedEvent;
}

FOnHoyoBattleSelectedEnemyChanged& UHoyoBattleFlowSubsystem::OnSelectedEnemyChanged()
{
	return SelectedEnemyChangedEvent;
}

bool UHoyoBattleFlowSubsystem::SpawnEncounterEnemies()
{
	if (!CurrentEncounter || !GetWorld())
	{
		return false;
	}

	const TArray<FHoyoBattleEnemyEntry>& EnemyEntries = CurrentEncounter->GetEnemyEntries();
	if (EnemyEntries.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("UHoyoBattleFlowSubsystem::SpawnEncounterEnemies found no enemy entries for encounter '%s'."), *CurrentEncounter->GetEncounterId().ToString());
		return true;
	}

	UWorld* World = GetWorld();
	bool bSpawnedAtLeastOneEnemy = false;

	for (const FHoyoBattleEnemyEntry& EnemyEntry : EnemyEntries)
	{
		UClass* EnemyClass = EnemyEntry.EnemyClass.LoadSynchronous();
		if (!EnemyClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("UHoyoBattleFlowSubsystem::SpawnEncounterEnemies skipped enemy '%s' because EnemyClass is null."), *EnemyEntry.EnemyId.ToString());
			continue;
		}

		const FTransform SpawnTransform = EnemyEntry.RelativeTransform * CurrentEncounterOriginTransform;
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* SpawnedEnemy = World->SpawnActor<AActor>(EnemyClass, SpawnTransform, SpawnParameters);
		if (!SpawnedEnemy)
		{
			UE_LOG(LogTemp, Warning, TEXT("UHoyoBattleFlowSubsystem::SpawnEncounterEnemies failed to spawn enemy '%s'."), *EnemyEntry.EnemyId.ToString());
			continue;
		}

		if (!SpawnedEnemy->GetClass()->ImplementsInterface(UHoyoEnemyInterface::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Spawned enemy actor '%s' does not implement IHoyoEnemyInterface and will be destroyed."), *GetNameSafe(SpawnedEnemy));
			SpawnedEnemy->Destroy();
			continue;
		}

		if (AHoyoBattleEnemyActor* BattleEnemyActor = Cast<AHoyoBattleEnemyActor>(SpawnedEnemy))
		{
			BattleEnemyActor->InitializeFromEnemyEntry(EnemyEntry);
		}

		SpawnedEncounterEnemies.Add(SpawnedEnemy);
		RegisterBattleEnemy(SpawnedEnemy);
		bSpawnedAtLeastOneEnemy = true;
	}

	return bSpawnedAtLeastOneEnemy || EnemyEntries.IsEmpty();
}

void UHoyoBattleFlowSubsystem::ResetBattleSession()
{
	for (const TWeakObjectPtr<AActor>& SpawnedEnemyPtr : SpawnedEncounterEnemies)
	{
		if (AActor* SpawnedEnemy = SpawnedEnemyPtr.Get())
		{
			SpawnedEnemy->Destroy();
		}
	}

	SpawnedEncounterEnemies.Reset();
	ActiveBattleEnemies.Reset();
	SetHoveredEnemy(nullptr);
	SetSelectedEnemy(nullptr);

	if (CurrentEncounter)
	{
		CurrentEncounter = nullptr;
		EncounterChangedEvent.Broadcast(nullptr);
	}

	CurrentEncounterOriginTransform = FTransform::Identity;
	SetFlowState(EHoyoBattleFlowState::Exploration);
}

void UHoyoBattleFlowSubsystem::PruneInvalidBattleEnemies()
{
	ActiveBattleEnemies.RemoveAll([](const TWeakObjectPtr<AActor>& EnemyPtr)
	{
		return !EnemyPtr.IsValid();
	});
}

void UHoyoBattleFlowSubsystem::SetFlowState(EHoyoBattleFlowState NewState)
{
	if (FlowState == NewState)
	{
		return;
	}

	const EHoyoBattleFlowState PreviousState = FlowState;
	FlowState = NewState;
	FlowStateChangedEvent.Broadcast(PreviousState, FlowState);
}
