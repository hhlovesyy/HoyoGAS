#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "SurvivorPlayerState.generated.h"

class USurvivorAbilitySystemComponent;
class USurvivorAttributeSet;
class USurvivorCardLoadoutComponent;
class UAbilitySystemComponent;

UCLASS()
class HOYOGAS_API ASurvivorPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASurvivorPlayerState();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	USurvivorAbilitySystemComponent* GetSurvivorAbilitySystemComponent() const;
	const USurvivorAttributeSet* GetSurvivorAttributeSet() const;
	USurvivorCardLoadoutComponent* GetCardLoadoutComponent() const;
	int32 GetSurvivorLevel() const;
	void SetSurvivorLevel(int32 NewLevel);
	float GetCurrentExperience() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TObjectPtr<USurvivorAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TObjectPtr<USurvivorAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TObjectPtr<USurvivorCardLoadoutComponent> CardLoadoutComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "SurvivorArena")
	int32 SurvivorLevel = 1;
};
