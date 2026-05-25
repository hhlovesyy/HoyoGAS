// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Battle/HoyoBattleTypes.h"
#include "HoyoBattleEncounterDefinition.generated.h"

UCLASS(BlueprintType)
class HOYOGAS_API UHoyoBattleEncounterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Battle")
	FName GetEncounterId() const { return EncounterId; }

	UFUNCTION(BlueprintPure, Category = "Battle")
	FName GetBattleScreenTag() const { return BattleScreenTag; }

	UFUNCTION(BlueprintPure, Category = "Battle")
	const TArray<FHoyoBattleEnemyEntry>& GetEnemyEntries() const { return EnemyEntries; }

	UFUNCTION(BlueprintPure, Category = "Battle")
	bool ShouldReturnToExplorationAfterBattle() const { return bReturnToExplorationAfterBattle; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	FName EncounterId = TEXT("Battle.Default");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	FName BattleScreenTag = TEXT("BattleScreen");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	TArray<FHoyoBattleEnemyEntry> EnemyEntries;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	bool bReturnToExplorationAfterBattle = true;
};
