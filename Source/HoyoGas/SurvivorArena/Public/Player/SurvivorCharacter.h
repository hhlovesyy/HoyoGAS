#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "SurvivorCharacter.generated.h"

class UAbilitySystemComponent;
class USurvivorAbilitySystemComponent;

UCLASS()
class HOYOGAS_API ASurvivorCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASurvivorCharacter();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	void InitAbilityActorInfo();

	UPROPERTY(Transient)
	TObjectPtr<USurvivorAbilitySystemComponent> CachedAbilitySystemComponent;
};
