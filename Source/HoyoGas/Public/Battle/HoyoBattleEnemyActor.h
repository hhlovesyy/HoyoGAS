// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Battle/HoyoBattleTypes.h"
#include "Character/HoyoEnemyInterface.h"
#include "GameFramework/Actor.h"
#include "HoyoBattleEnemyActor.generated.h"

class UHoyoBattleFlowSubsystem;
class UPrimitiveComponent;
class USceneComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;

UCLASS()
class HOYOGAS_API AHoyoBattleEnemyActor : public AActor, public IHoyoEnemyInterface
{
	GENERATED_BODY()

public:
	AHoyoBattleEnemyActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	void InitializeFromEnemyEntry(const FHoyoBattleEnemyEntry& EnemyEntry);

	virtual bool CanBeTargeted_Implementation() const override;
	virtual FVector GetTargetLocation_Implementation() const override;
	virtual void SetEnemyHighlighted_Implementation(bool bHighlighted) override;
	virtual bool IsEnemyAlive_Implementation() const override;
	virtual FName GetEnemyId_Implementation() const override;
	virtual FText GetEnemyDisplayName_Implementation() const override;
	virtual int32 GetEnemyLevel_Implementation() const override;

protected:
	void RefreshVisualState();
	void ApplyHighlightToComponent(UPrimitiveComponent* PrimitiveComponent, bool bHighlighted) const;
	UHoyoBattleFlowSubsystem* ResolveBattleFlowSubsystem() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> FallbackMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Battle|Identity")
	FName EnemyId = TEXT("Enemy.Default");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Battle|Identity")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Battle|Identity", meta = (ClampMin = "1"))
	int32 EnemyLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Battle|State")
	bool bIsTargetable = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Battle|State")
	bool bIsAlive = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Battle|Targeting")
	FName TargetSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Battle|Targeting", meta = (ClampMin = "0.0"))
	float TargetLocationZOffset = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Battle|Highlight", meta = (ClampMin = "1", ClampMax = "255"))
	int32 HighlightStencilValue = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Battle|Debug")
	FLinearColor FallbackMeshColor = FLinearColor(0.75f, 0.22f, 0.3f, 1.0f);

private:
	bool bIsHighlighted = false;
};
