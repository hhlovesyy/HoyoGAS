// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "HoyoAttributeSet.generated.h"

#ifndef ATTRIBUTE_ACCESSORS
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
#endif

/**
 * Core GAS attributes used by characters and enemies.
 *
 * Element-specific damage bonuses and resistances are intentionally not stored here.
 * Those should come from tags, config tables, equipment/buff data, or damage execution logic.
 */
UCLASS()
class HOYOGAS_API UHoyoAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UHoyoAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	// Genshin: Current HP / 生命值. Changed directly by damage and healing.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health, Category="Vital Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UHoyoAttributeSet, Health)

	// Genshin: Max HP / 生命值上限. Upper bound for Health.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHealth, Category="Vital Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UHoyoAttributeSet, MaxHealth)

	// Genshin: ATK / 攻击力. Main base value for skill multipliers and normal damage.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_AttackPower, Category="Combat Attributes")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UHoyoAttributeSet, AttackPower)

	// Genshin: DEF / 防御力. Usually used by the defender in the defense damage formula.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Defense, Category="Combat Attributes")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UHoyoAttributeSet, Defense)

	// Genshin: Elemental Mastery / 元素精通. Affects Vaporize, Melt, and later transformative reactions.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_ElementalMastery, Category="Combat Attributes")
	FGameplayAttributeData ElementalMastery;
	ATTRIBUTE_ACCESSORS(UHoyoAttributeSet, ElementalMastery)

	// Genshin: Character or monster level / 等级. Used by defense formulas and reaction level scaling.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Level, Category="Combat Attributes")
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS(UHoyoAttributeSet, Level)

	// Genshin: CRIT Rate / 暴击率. Use 0.05 to represent 5%.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CritRate, Category="Combat Attributes")
	FGameplayAttributeData CritRate;
	ATTRIBUTE_ACCESSORS(UHoyoAttributeSet, CritRate)

	// Genshin: CRIT DMG / 暴击伤害. Use 0.50 to represent +50% critical damage.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CritDamage, Category="Combat Attributes")
	FGameplayAttributeData CritDamage;
	ATTRIBUTE_ACCESSORS(UHoyoAttributeSet, CritDamage)

	// Genshin: Current elemental energy / 元素能量. Current Burst energy value.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Energy, Category="Resource Attributes")
	FGameplayAttributeData Energy;
	ATTRIBUTE_ACCESSORS(UHoyoAttributeSet, Energy)

	// Genshin: Burst energy cost / 元素能量上限. Common values are 40/60/70/80/90.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxEnergy, Category="Resource Attributes")
	FGameplayAttributeData MaxEnergy;
	ATTRIBUTE_ACCESSORS(UHoyoAttributeSet, MaxEnergy)

	// Genshin: Energy Recharge / 元素充能效率. Use 1.0 to represent 100%.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_EnergyRecharge, Category="Resource Attributes")
	FGameplayAttributeData EnergyRecharge;
	ATTRIBUTE_ACCESSORS(UHoyoAttributeSet, EnergyRecharge)

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower) const;

	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& OldDefense) const;

	UFUNCTION()
	void OnRep_ElementalMastery(const FGameplayAttributeData& OldElementalMastery) const;

	UFUNCTION()
	void OnRep_Level(const FGameplayAttributeData& OldLevel) const;

	UFUNCTION()
	void OnRep_CritRate(const FGameplayAttributeData& OldCritRate) const;

	UFUNCTION()
	void OnRep_CritDamage(const FGameplayAttributeData& OldCritDamage) const;

	UFUNCTION()
	void OnRep_Energy(const FGameplayAttributeData& OldEnergy) const;

	UFUNCTION()
	void OnRep_MaxEnergy(const FGameplayAttributeData& OldMaxEnergy) const;

	UFUNCTION()
	void OnRep_EnergyRecharge(const FGameplayAttributeData& OldEnergyRecharge) const;

private:
	float ClampAttributeValue(const FGameplayAttribute& Attribute, float Value) const;
	void ClampHealthToMaxHealth();
	void ClampEnergyToMaxEnergy();
};
