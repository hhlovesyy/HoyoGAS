// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HoyoEnemyInterface.generated.h"

UINTERFACE(BlueprintType)
class HOYOGAS_API UHoyoEnemyInterface : public UInterface
{
	GENERATED_BODY()
};

class HOYOGAS_API IHoyoEnemyInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy")
	bool CanBeTargeted() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy")
	FVector GetTargetLocation() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy")
	void SetEnemyHighlighted(bool bHighlighted);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy")
	bool IsEnemyAlive() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy")
	FName GetEnemyId() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy")
	FText GetEnemyDisplayName() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy")
	int32 GetEnemyLevel() const;
};
