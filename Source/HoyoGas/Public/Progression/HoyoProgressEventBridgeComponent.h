// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "HoyoProgressEventBridgeComponent.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;

UCLASS( ClassGroup=(Progression), meta=(BlueprintSpawnableComponent) )
class HOYOGAS_API UHoyoProgressEventBridgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHoyoProgressEventBridgeComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void BindAbilitySystem();
	void UnbindAbilitySystem();
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void PublishProgressEvent(FGameplayTag EventTag) const;

	UPROPERTY(Transient)
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	FDelegateHandle HealthChangedHandle;
};
