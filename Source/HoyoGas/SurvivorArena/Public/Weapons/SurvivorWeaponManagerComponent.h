#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Components/ActorComponent.h"
#include "SurvivorWeaponManagerComponent.generated.h"

class UAbilitySystemComponent;
class USurvivorWeaponDefinition;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSurvivorWeaponsChanged, class USurvivorWeaponManagerComponent*);

USTRUCT()
struct FSurvivorGrantedWeaponEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USurvivorWeaponDefinition> WeaponDefinition = nullptr;

	FGameplayAbilitySpecHandle WeaponAbilityHandle;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorGrantedWeaponHandles
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Weapon")
	TObjectPtr<USurvivorWeaponDefinition> WeaponDefinition = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Weapon")
	FGameplayAbilitySpecHandle WeaponAbilityHandle;

	bool IsValid() const
	{
		return WeaponDefinition != nullptr && WeaponAbilityHandle.IsValid();
	}
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

	bool GrantWeaponAndCollect(USurvivorWeaponDefinition* WeaponDefinition, FSurvivorGrantedWeaponHandles& OutGrantedHandles);

	UFUNCTION(BlueprintCallable, Category = "Survivor|Weapon")
	bool RemoveWeapon(USurvivorWeaponDefinition* WeaponDefinition);

	bool RemoveWeaponByHandles(const FSurvivorGrantedWeaponHandles& GrantedHandles);

	UFUNCTION(BlueprintPure, Category = "Survivor|Weapon")
	bool HasWeapon(const USurvivorWeaponDefinition* WeaponDefinition) const;

	UFUNCTION(BlueprintPure, Category = "Survivor|Weapon")
	int32 GetGrantedWeaponCount() const;

	FOnSurvivorWeaponsChanged& OnWeaponsChanged();

protected:
	UAbilitySystemComponent* GetOwningAbilitySystemComponent() const;
	FSurvivorGrantedWeaponEntry* FindGrantedWeaponEntry(USurvivorWeaponDefinition* WeaponDefinition);
	const FSurvivorGrantedWeaponEntry* FindGrantedWeaponEntry(const USurvivorWeaponDefinition* WeaponDefinition) const;
	void RemoveAllGrantedWeapons();
	void BroadcastWeaponsChanged();

	UPROPERTY()
	TArray<FSurvivorGrantedWeaponEntry> GrantedWeapons;

	FOnSurvivorWeaponsChanged WeaponsChangedEvent;
};
