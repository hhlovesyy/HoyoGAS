// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Battle/HoyoBattleTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "HoyoBattleFlowSubsystem.generated.h"

class AActor;
class UHoyoBattleEncounterDefinition;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHoyoBattleFlowStateChanged, EHoyoBattleFlowState, EHoyoBattleFlowState);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHoyoBattleEncounterChanged, const UHoyoBattleEncounterDefinition*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHoyoBattleHoveredEnemyChanged, AActor*, AActor*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHoyoBattleSelectedEnemyChanged, AActor*, AActor*);

UCLASS()
class HOYOGAS_API UHoyoBattleFlowSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool RequestEnterBattle(UHoyoBattleEncounterDefinition* EncounterDefinition, const FTransform& EncounterOriginTransform);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool EnterTargetSelectionState();

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool BeginBattleExit();

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void CompleteBattleExit();

	UFUNCTION(BlueprintPure, Category = "Battle")
	EHoyoBattleFlowState GetFlowState() const;

	UFUNCTION(BlueprintPure, Category = "Battle")
	bool IsBattleActive() const;

	UFUNCTION(BlueprintPure, Category = "Battle")
	UHoyoBattleEncounterDefinition* GetCurrentEncounter() const;

	UFUNCTION(BlueprintPure, Category = "Battle")
	FName GetCurrentEncounterId() const;

	UFUNCTION(BlueprintPure, Category = "Battle")
	FTransform GetCurrentEncounterOriginTransform() const;

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void RegisterBattleEnemy(AActor* EnemyActor);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void UnregisterBattleEnemy(AActor* EnemyActor);

	UFUNCTION(BlueprintPure, Category = "Battle")
	void GetActiveBattleEnemies(TArray<AActor*>& OutEnemies) const;

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void SetHoveredEnemy(AActor* EnemyActor);

	UFUNCTION(BlueprintPure, Category = "Battle")
	AActor* GetHoveredEnemy() const;

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void SetSelectedEnemy(AActor* EnemyActor);

	UFUNCTION(BlueprintPure, Category = "Battle")
	AActor* GetSelectedEnemy() const;

	FOnHoyoBattleFlowStateChanged& OnFlowStateChanged();
	FOnHoyoBattleEncounterChanged& OnEncounterChanged();
	FOnHoyoBattleHoveredEnemyChanged& OnHoveredEnemyChanged();
	FOnHoyoBattleSelectedEnemyChanged& OnSelectedEnemyChanged();

private:
	bool SpawnEncounterEnemies();
	void ResetBattleSession();
	void PruneInvalidBattleEnemies();
	void SetFlowState(EHoyoBattleFlowState NewState);

	UPROPERTY(Transient)
	TObjectPtr<UHoyoBattleEncounterDefinition> CurrentEncounter = nullptr;

	FTransform CurrentEncounterOriginTransform = FTransform::Identity;
	EHoyoBattleFlowState FlowState = EHoyoBattleFlowState::Exploration;
	TArray<TWeakObjectPtr<AActor>> ActiveBattleEnemies;
	TArray<TWeakObjectPtr<AActor>> SpawnedEncounterEnemies;
	TWeakObjectPtr<AActor> HoveredEnemyActor;
	TWeakObjectPtr<AActor> SelectedEnemyActor;

	FOnHoyoBattleFlowStateChanged FlowStateChangedEvent;
	FOnHoyoBattleEncounterChanged EncounterChangedEvent;
	FOnHoyoBattleHoveredEnemyChanged HoveredEnemyChangedEvent;
	FOnHoyoBattleSelectedEnemyChanged SelectedEnemyChangedEvent;
};
