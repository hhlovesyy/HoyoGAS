// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HoyoBattleTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class EHoyoBattleFlowState : uint8
{
	Exploration,
	EnteringBattle,
	TargetSelection,
	ExitingBattle
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoBattleEnemyEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	FName EnemyId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	TSoftClassPtr<AActor> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	FTransform RelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 EnemyLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	FName SpawnPointTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	FText DisplayName;
};
