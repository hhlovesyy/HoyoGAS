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
	int32 GetSurvivorLevelValue() const;
	const FText& GetWeaponCountText() const;
	const FText& GetGoldText() const;
	const FText& GetExperienceText() const;
	float GetCurrentExperienceValue() const;
	bool HasExperienceProgressTarget() const;
	float GetExperienceProgressTargetValue() const;
	float GetExperienceProgressPercent() const;
	int32 GetGoldDelta() const;
	float GetExperienceDelta() const;
	int32 GetGoldChangeEventId() const;
	int32 GetExperienceChangeEventId() const;
	float GetExperienceLevelUpClearedAmount() const;
	int32 GetExperienceLevelUpEventId() const;
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
	void RefreshEconomySnapshot(bool bAllowDeltaEvents);
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

	int32 CachedSurvivorLevel = 1;
	int32 CachedGold = 0;
	float CachedExperience = 0.0f;
	bool bHasExperienceProgressTarget = false;
	float ExperienceProgressTargetValue = 0.0f;
	float ExperienceProgressPercent = 0.0f;
	int32 GoldDelta = 0;
	float ExperienceDelta = 0.0f;
	int32 GoldChangeEventId = 0;
	int32 ExperienceChangeEventId = 0;
	float ExperienceLevelUpClearedAmount = 0.0f;
	int32 ExperienceLevelUpEventId = 0;
	bool bHasEconomySnapshot = false;

	UPROPERTY(Transient)
	FText CardSlotsText;

	UPROPERTY(Transient)
	FText EquippedCardsText;

	UPROPERTY(Transient)
	FText CardTagsText;

	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
};
