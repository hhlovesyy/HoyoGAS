#pragma once

#include "CoreMinimal.h"
#include "Stores/MyUIStoreBase.h"
#include "PlayerVitalsUIStore.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;

UCLASS(BlueprintType)
class HOYOGAS_API UPlayerVitalsUIStore : public UUIStoreBase
{
	GENERATED_BODY()

public:
	virtual void BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState) override;
	virtual void UnbindFromPlayerContext() override;

	float GetHealth() const;
	float GetMaxHealth() const;
	float GetHealthPercent() const;
	const FText& GetHealthText() const;

	float GetEnergy() const;
	float GetMaxEnergy() const;
	float GetEnergyPercent() const;
	const FText& GetEnergyText() const;
	const FText& GetEnergyRechargeText() const;

private:
	void BindAbilitySystem(UAbilitySystemComponent* InAbilitySystemComponent);
	void UnbindAbilitySystem();
	void RefreshFromAbilitySystem();
	void RebuildDisplayValues();

	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	void HandleEnergyChanged(const FOnAttributeChangeData& Data);
	void HandleMaxEnergyChanged(const FOnAttributeChangeData& Data);
	void HandleEnergyRechargeChanged(const FOnAttributeChangeData& Data);

	UPROPERTY(Transient)
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
	FDelegateHandle EnergyChangedHandle;
	FDelegateHandle MaxEnergyChangedHandle;
	FDelegateHandle EnergyRechargeChangedHandle;

	UPROPERTY(Transient)
	float Health = 0.0f;

	UPROPERTY(Transient)
	float MaxHealth = 0.0f;

	UPROPERTY(Transient)
	float HealthPercent = 0.0f;

	UPROPERTY(Transient)
	FText HealthText;

	UPROPERTY(Transient)
	float Energy = 0.0f;

	UPROPERTY(Transient)
	float MaxEnergy = 0.0f;

	UPROPERTY(Transient)
	float EnergyPercent = 0.0f;

	UPROPERTY(Transient)
	FText EnergyText;

	UPROPERTY(Transient)
	float EnergyRecharge = 1.0f;

	UPROPERTY(Transient)
	FText EnergyRechargeText;
};
