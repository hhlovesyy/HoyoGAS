#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SurvivorGameplayAbilityBase.generated.h"

class USurvivorAbilitySystemComponent;

UCLASS(Abstract)
class HOYOGAS_API USurvivorGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena")
	FName AbilityId = NAME_None;

protected:
	UFUNCTION(BlueprintPure, Category = "SurvivorArena")
	USurvivorAbilitySystemComponent* GetSurvivorASCFromActorInfo() const;
};
