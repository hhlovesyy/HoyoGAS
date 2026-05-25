// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/HoyoCharacterBase.h"
#include "Character/HoyoEnemyInterface.h"
#include "HoyoEnemy.generated.h"

/**
 * 
 */
UCLASS()
class HOYOGAS_API AHoyoEnemy : public AHoyoCharacterBase, public IHoyoEnemyInterface
{
	GENERATED_BODY()

public:
	AHoyoEnemy();

	virtual bool CanBeTargeted_Implementation() const override;
	virtual FVector GetTargetLocation_Implementation() const override;
	virtual void SetEnemyHighlighted_Implementation(bool bHighlighted) override;
	virtual bool IsEnemyAlive_Implementation() const override;
	virtual FName GetEnemyId_Implementation() const override;
	virtual FText GetEnemyDisplayName_Implementation() const override;
	virtual int32 GetEnemyLevel_Implementation() const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FName EnemyId = TEXT("Enemy.Default");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "1"))
	int32 EnemyLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	bool bIsTargetable = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	bool bIsAlive = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Targeting")
	FName TargetSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Targeting", meta = (ClampMin = "0.0"))
	float TargetLocationZOffset = 88.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Highlight", meta = (ClampMin = "1", ClampMax = "255"))
	int32 HighlightStencilValue = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Highlight")
	bool bHighlightWeapon = false;

private:
	void ApplyHighlightState(bool bHighlighted);

	bool bIsHighlighted = false;
};
