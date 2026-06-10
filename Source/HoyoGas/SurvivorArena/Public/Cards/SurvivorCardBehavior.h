#pragma once

#include "CoreMinimal.h"
#include "Cards/SurvivorCardRuntimeData.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "Cards/SurvivorCardRuntimeTypes.h"
#include "SurvivorCardBehavior.generated.h"

class UAbilitySystemComponent;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class HOYOGAS_API USurvivorCardBehavior : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Cards")
	bool bReceivesBehaviorTick = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Cards")
	FName RuntimeStateKey = NAME_None;

	UFUNCTION(BlueprintNativeEvent, Category = "SurvivorArena|Cards")
	bool OnCardEquipped(const FSurvivorCardBehaviorContext& Context, FSurvivorAppliedCardHandles& OutAppliedHandles) const;

	UFUNCTION(BlueprintNativeEvent, Category = "SurvivorArena|Cards")
	void OnCardUnequipped(const FSurvivorCardBehaviorContext& Context, const FSurvivorAppliedCardHandles& AppliedHandles) const;

	UFUNCTION(BlueprintNativeEvent, Category = "SurvivorArena|Cards")
	void OnRunStarted(const FSurvivorCardBehaviorContext& Context) const;

	UFUNCTION(BlueprintNativeEvent, Category = "SurvivorArena|Cards")
	void OnEnemyKilled(const FSurvivorCardEnemyKillContext& Context) const;

	UFUNCTION(BlueprintNativeEvent, Category = "SurvivorArena|Cards")
	void OnCardStackChanged(const FSurvivorCardBehaviorContext& Context, int32 OldStackCount, int32 NewStackCount) const;

	UFUNCTION(BlueprintNativeEvent, Category = "SurvivorArena|Cards")
	void TickBehavior(const FSurvivorCardBehaviorContext& Context, float DeltaSeconds) const;

protected:
	const FSurvivorCardRuntimeInstance* ResolveRuntimeInstance(const FSurvivorCardBehaviorContext& Context) const;
	FSurvivorCardRuntimeState* ResolveMutableRuntimeState(const FSurvivorCardBehaviorContext& Context) const;
	USurvivorCardRuntimeData* ResolveRuntimeData(const FSurvivorCardBehaviorContext& Context, TSubclassOf<USurvivorCardRuntimeData> RuntimeDataClass) const;
	void ExecuteGameplayCue(
		const FSurvivorCardBehaviorContext& Context,
		UAbilitySystemComponent* SourceASC,
		FGameplayTag GameplayCueTag,
		float RawMagnitude = 0.0f,
		const FGameplayEffectContextHandle* EffectContext = nullptr) const;
	FName ResolveRuntimeStateKey() const;
};
