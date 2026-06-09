#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "SurvivorRunEconomyComponent.generated.h"

class UAbilitySystemComponent;
class UUIStoreBase;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSurvivorEconomyChanged, class USurvivorRunEconomyComponent*);

UCLASS(ClassGroup = (Survivor), meta = (BlueprintSpawnableComponent))
class HOYOGAS_API USurvivorRunEconomyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USurvivorRunEconomyComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	int32 GetGold() const;
	float GetExperience() const;

	void AddGold(int32 DeltaGold);
	bool SpendGold(int32 DeltaGold);
	void AddExperience(float DeltaExperience);

	FOnSurvivorEconomyChanged& OnEconomyChanged();

protected:
	void BindAbilitySystem(UAbilitySystemComponent* InAbilitySystemComponent);
	void HandleExperienceChanged(const FOnAttributeChangeData& ChangeData);

	UFUNCTION()
	void OnRep_Gold();

	UPROPERTY(Transient)
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Gold, Category = "Survivor|Economy")
	int32 Gold = 0;

	FDelegateHandle ExperienceChangedHandle;
	FOnSurvivorEconomyChanged EconomyChangedEvent;
};
