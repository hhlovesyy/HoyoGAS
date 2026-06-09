#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GAS/SurvivorAbilitySet.h"
#include "Weapons/SurvivorWeaponManagerComponent.h"
#include "SurvivorRunInventoryComponent.generated.h"

class UAbilitySystemComponent;
class USurvivorWeaponDefinition;
class USurvivorWeaponManagerComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSurvivorRunInventoryChanged, class USurvivorRunInventoryComponent*);

UCLASS(ClassGroup = (Survivor), meta = (BlueprintSpawnableComponent))
class HOYOGAS_API USurvivorRunInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USurvivorRunInventoryComponent();

	virtual void BeginPlay() override;

	bool GrantStartingAbilitySet(USurvivorAbilitySet* AbilitySet, UObject* SourceObject);
	bool GrantStartingWeapon(USurvivorWeaponDefinition* WeaponDefinition);
	void ClearStartingLoadout();

	UFUNCTION(BlueprintPure, Category = "SurvivorArena|Loadout")
	int32 GetGrantedStartingWeaponCount() const;

	UFUNCTION(BlueprintPure, Category = "SurvivorArena|Loadout")
	int32 GetGrantedStartingAbilitySetCount() const;

	FOnSurvivorRunInventoryChanged& OnInventoryChanged();

protected:
	UAbilitySystemComponent* ResolveOwnerASC() const;
	USurvivorWeaponManagerComponent* ResolveOwnerWeaponManager() const;
	void BroadcastInventoryChanged();

	UPROPERTY(Transient)
	TArray<FSurvivorGrantedAbilitySetHandles> GrantedStartingAbilitySetHandles;

	UPROPERTY(Transient)
	TArray<FSurvivorGrantedWeaponHandles> GrantedStartingWeapons;

	FOnSurvivorRunInventoryChanged InventoryChangedEvent;
};
