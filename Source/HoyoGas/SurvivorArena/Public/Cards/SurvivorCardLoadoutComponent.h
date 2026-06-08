#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GAS/SurvivorAbilitySet.h"
#include "GameplayTagContainer.h"
#include "SurvivorCardLoadoutComponent.generated.h"

class UAbilitySystemComponent;
class USurvivorCardDefinition;

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorAppliedCardHandles
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TArray<FActiveGameplayEffectHandle> ActiveEffectHandles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TArray<FGameplayAbilitySpecHandle> GrantedAbilitySpecHandles;

	bool IsEmpty() const;
	void Reset();
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorEquippedCardEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TObjectPtr<USurvivorCardDefinition> CardDefinition = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	int32 StackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TArray<FSurvivorAppliedCardHandles> AppliedStacks;
};

UCLASS(ClassGroup=(Survivor), meta=(BlueprintSpawnableComponent))
class HOYOGAS_API USurvivorCardLoadoutComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USurvivorCardLoadoutComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "SurvivorArena|Cards")
	bool EquipCard(USurvivorCardDefinition* CardDefinition);

	UFUNCTION(BlueprintCallable, Category = "SurvivorArena|Cards")
	bool UnequipCard(USurvivorCardDefinition* CardDefinition);

	UFUNCTION(BlueprintCallable, Category = "SurvivorArena|Cards")
	void ClearCards();

	UFUNCTION(BlueprintPure, Category = "SurvivorArena|Cards")
	int32 GetUsedCardSlots() const;

	UFUNCTION(BlueprintPure, Category = "SurvivorArena|Cards")
	bool HasFreeCardSlot() const;

	UFUNCTION(BlueprintPure, Category = "SurvivorArena|Cards")
	const FGameplayTagContainer& GetAggregatedCardTags() const;

	UFUNCTION(BlueprintPure, Category = "SurvivorArena|Cards")
	const TArray<FSurvivorEquippedCardEntry>& GetEquippedCardEntries() const;

	UFUNCTION(BlueprintCallable, Category = "SurvivorArena|Cards")
	void RecalculateAggregatedCardTags();

	UFUNCTION(BlueprintCallable, Category = "SurvivorArena|Cards")
	void PrintCardTagSummary() const;

protected:
	UAbilitySystemComponent* ResolveOwnerASC() const;
	bool ApplyCardStack(USurvivorCardDefinition* CardDefinition, FSurvivorAppliedCardHandles& OutAppliedHandles);
	void RemoveAppliedHandles(const FSurvivorAppliedCardHandles& AppliedHandles);
	void EmitCardDebugMessage(const FString& Message, const FColor& Color) const;
	FString BuildCardTagSummaryString() const;
	FSurvivorEquippedCardEntry* FindCardEntry(USurvivorCardDefinition* CardDefinition);
	const FSurvivorEquippedCardEntry* FindCardEntry(USurvivorCardDefinition* CardDefinition) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena|Cards")
	int32 MaxCardSlots = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena|Cards")
	bool bEnableOnScreenCardDebug = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena|Cards")
	TArray<FSurvivorEquippedCardEntry> EquippedCardEntries;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena|Cards")
	FGameplayTagContainer AggregatedCardTags;
};
