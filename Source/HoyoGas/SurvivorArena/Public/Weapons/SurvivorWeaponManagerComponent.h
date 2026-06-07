#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Components/ActorComponent.h"
#include "SurvivorWeaponManagerComponent.generated.h"

class UAbilitySystemComponent;
class USurvivorWeaponDefinition;

USTRUCT()
struct FSurvivorGrantedWeaponEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USurvivorWeaponDefinition> WeaponDefinition = nullptr;

	FGameplayAbilitySpecHandle WeaponAbilityHandle;
};

UCLASS(ClassGroup = (Survivor), meta = (BlueprintSpawnableComponent))
class HOYOGAS_API USurvivorWeaponManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USurvivorWeaponManagerComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Survivor|Weapon")
	bool GrantWeapon(USurvivorWeaponDefinition* WeaponDefinition);

	UFUNCTION(BlueprintCallable, Category = "Survivor|Weapon")
	bool RemoveWeapon(USurvivorWeaponDefinition* WeaponDefinition);

	UFUNCTION(BlueprintPure, Category = "Survivor|Weapon")
	bool HasWeapon(const USurvivorWeaponDefinition* WeaponDefinition) const;

protected:
	UAbilitySystemComponent* GetOwningAbilitySystemComponent() const;
	FSurvivorGrantedWeaponEntry* FindGrantedWeaponEntry(USurvivorWeaponDefinition* WeaponDefinition);
	const FSurvivorGrantedWeaponEntry* FindGrantedWeaponEntry(const USurvivorWeaponDefinition* WeaponDefinition) const;
	void RemoveAllGrantedWeapons();

	UPROPERTY()
	TArray<FSurvivorGrantedWeaponEntry> GrantedWeapons;
};
