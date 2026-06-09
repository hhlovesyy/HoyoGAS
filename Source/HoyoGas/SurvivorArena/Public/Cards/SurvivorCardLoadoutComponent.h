#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Cards/SurvivorCardRuntimeTypes.h"
#include "GAS/SurvivorAbilitySet.h"
#include "GameplayTagContainer.h"
#include "Weapons/SurvivorWeaponManagerComponent.h"
#include "SurvivorCardLoadoutComponent.generated.h"

class UAbilitySystemComponent;
class USurvivorCardDefinition;
class USurvivorWeaponManagerComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSurvivorLoadoutChanged, class USurvivorCardLoadoutComponent*);

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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool GrantStartingAbilitySet(USurvivorAbilitySet* AbilitySet, UObject* SourceObject);
	bool GrantStartingWeapon(USurvivorWeaponDefinition* WeaponDefinition, USurvivorWeaponManagerComponent* WeaponManager);
	void ClearStartingLoadout(USurvivorWeaponManagerComponent* WeaponManager);
	void NotifyRunStarted();
	void NotifyEnemyKilled(AActor* KilledEnemyActor, int32 KillCountDelta = 1);

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

	UFUNCTION(BlueprintPure, Category = "SurvivorArena|Loadout")
	int32 GetGrantedStartingWeaponCount() const;

	UFUNCTION(BlueprintPure, Category = "SurvivorArena|Loadout")
	int32 GetGrantedStartingAbilitySetCount() const;

	UFUNCTION(BlueprintPure, Category = "SurvivorArena|Cards")
	int32 GetMaxCardSlots() const;

	UFUNCTION(BlueprintPure, Category = "SurvivorArena|Cards")
	FText GetEquippedCardsSummaryText() const;

	UFUNCTION(BlueprintPure, Category = "SurvivorArena|Cards")
	FText GetAggregatedCardTagsSummaryText() const;

	FOnSurvivorLoadoutChanged& OnLoadoutChanged();

protected:
	UAbilitySystemComponent* ResolveOwnerASC() const;
	FSurvivorCardBehaviorContext BuildBehaviorContext(USurvivorCardDefinition* CardDefinition, int32 StackCount) const;
	bool ApplyCardStack(USurvivorCardDefinition* CardDefinition, FSurvivorAppliedCardHandles& OutAppliedHandles);
	void RemoveAppliedHandles(const FSurvivorAppliedCardHandles& AppliedHandles);
	void RefreshBehaviorTickState();
	void BroadcastCardStackChanged(USurvivorCardDefinition* CardDefinition, int32 OldStackCount, int32 NewStackCount);
	void BroadcastLoadoutChanged();
	void EmitCardDebugMessage(const FString& Message, const FColor& Color) const;
	FString BuildCardTagSummaryString() const;
	FString BuildEquippedCardsSummaryString() const;
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

	UPROPERTY(Transient)
	TArray<FSurvivorGrantedAbilitySetHandles> GrantedStartingAbilitySetHandles;

	UPROPERTY(Transient)
	TArray<FSurvivorGrantedWeaponHandles> GrantedStartingWeapons;

	FOnSurvivorLoadoutChanged LoadoutChangedEvent;
};
