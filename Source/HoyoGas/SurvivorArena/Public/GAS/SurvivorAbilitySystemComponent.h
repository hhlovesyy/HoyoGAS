#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "SurvivorAbilitySystemComponent.generated.h"

UCLASS()
class HOYOGAS_API USurvivorAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
};
