#pragma once

#include "CoreMinimal.h"
#include "Stores/MyUIStoreBase.h"
#include "SurvivorHUDStore.generated.h"

class UAbilitySystemComponent;
class USurvivorCardLoadoutComponent;
class USurvivorRunEconomyComponent;
class USurvivorWeaponManagerComponent;
struct FOnAttributeChangeData;

UCLASS(BlueprintType)
class HOYOGAS_API USurvivorHUDStore : public UUIStoreBase
{
	GENERATED_BODY()

public:
	virtual void BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState) override;
	virtual void UnbindFromPlayerContext() override;

	const FText& GetHealthText() const;
	float GetHealthPercent() const;
	const FText& GetLevelText() const;
	const FText& GetWeaponCountText() const;
	const FText& GetGoldText() const;
	const FText& GetExperienceText() const;
	const FText& GetCardSlotsText() const;
	const FText& GetEquippedCardsText() const;
	const FText& GetCardTagsText() const;

protected:
	void BindAbilitySystem(UAbilitySystemComponent* InAbilitySystemComponent);
	void UnbindAbilitySystem();
	void BindLoadoutComponent(USurvivorCardLoadoutComponent* InLoadoutComponent);
	void UnbindLoadoutComponent();
	void BindRunEconomyComponent(USurvivorRunEconomyComponent* InRunEconomyComponent);
	void UnbindRunEconomyComponent();
	void BindWeaponManager(USurvivorWeaponManagerComponent* InWeaponManager);
	void UnbindWeaponManager();
	void RefreshAll();
	void RebuildDisplayValues();

	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	void HandleLoadoutChanged(USurvivorCardLoadoutComponent* ChangedComponent);
	void HandleEconomyChanged(USurvivorRunEconomyComponent* ChangedComponent);
	void HandleWeaponsChanged(USurvivorWeaponManagerComponent* ChangedComponent);

	UPROPERTY(Transient)
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<USurvivorCardLoadoutComponent> LoadoutComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<USurvivorRunEconomyComponent> RunEconomyComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<USurvivorWeaponManagerComponent> WeaponManagerComponent;

	float Health = 0.0f;
	float MaxHealth = 0.0f;

	UPROPERTY(Transient)
	FText HealthText;

	UPROPERTY(Transient)
	float HealthPercent = 0.0f;

	UPROPERTY(Transient)
	FText LevelText;

	UPROPERTY(Transient)
	FText WeaponCountText;

	UPROPERTY(Transient)
	FText GoldText;

	UPROPERTY(Transient)
	FText ExperienceText;

	UPROPERTY(Transient)
	FText CardSlotsText;

	UPROPERTY(Transient)
	FText EquippedCardsText;

	UPROPERTY(Transient)
	FText CardTagsText;

	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
};
