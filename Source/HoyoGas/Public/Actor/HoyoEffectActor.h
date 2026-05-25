// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "HoyoEffectActor.generated.h"

class UGameplayEffect;

UCLASS()
class HOYOGAS_API AHoyoEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AHoyoEffectActor();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	TSubclassOf<UGameplayEffect> InstantGameplayEffectClass; //不需要实例，我们需要的是“配方（Class）” 限制你在 UE 编辑器的属性面板里，只能选择 UGameplayEffect 或它的子类（蓝图）。
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	TSubclassOf<UGameplayEffect> DurationGameplayEffectClass;
	
	UFUNCTION(BlueprintCallable, Category="Applied Effects")
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void BroadcastHoyoEffectPickedUp();

private:
};
