#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "SurvivorAttributeSet.generated.h"

#ifndef ATTRIBUTE_ACCESSORS
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
#endif

UCLASS()
class HOYOGAS_API USurvivorAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	USurvivorAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "SurvivorArena|Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(USurvivorAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "SurvivorArena|Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(USurvivorAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeed, Category = "SurvivorArena|Attributes")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(USurvivorAttributeSet, MoveSpeed)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPower, Category = "SurvivorArena|Attributes")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(USurvivorAttributeSet, AttackPower)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackSpeed, Category = "SurvivorArena|Attributes")
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS(USurvivorAttributeSet, AttackSpeed)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PickupRadius, Category = "SurvivorArena|Attributes")
	FGameplayAttributeData PickupRadius;
	ATTRIBUTE_ACCESSORS(USurvivorAttributeSet, PickupRadius)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Experience, Category = "SurvivorArena|Attributes")
	FGameplayAttributeData Experience;
	ATTRIBUTE_ACCESSORS(USurvivorAttributeSet, Experience)

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_AttackSpeed(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_PickupRadius(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_Experience(const FGameplayAttributeData& OldValue) const;

private:
	float ClampAttributeValue(const FGameplayAttribute& Attribute, float Value) const;
	void ClampHealthToMaxHealth();
};
