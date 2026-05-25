// Copyright Epic Games, Inc. All Rights Reserved.

#include "MYGasLearnHealthAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UMYGasLearnHealthAttributeSet::UMYGasLearnHealthAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
}

void UMYGasLearnHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMYGasLearnHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMYGasLearnHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UMYGasLearnHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	NewValue = ClampAttributeValue(Attribute, NewValue);
}

void UMYGasLearnHealthAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		BroadcastHealthChanged(OldValue, NewValue);
	}
}

void UMYGasLearnHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetMaxHealth(FMath::Max(GetMaxHealth(), 0.0f));
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
}

float UMYGasLearnHealthAttributeSet::ClampAttributeValue(const FGameplayAttribute& Attribute, float Value) const
{
	if (Attribute == GetHealthAttribute())
	{
		return FMath::Clamp(Value, 0.0f, GetMaxHealth());
	}

	if (Attribute == GetMaxHealthAttribute())
	{
		return FMath::Max(Value, 0.0f);
	}

	return Value;
}

void UMYGasLearnHealthAttributeSet::BroadcastHealthChanged(float OldValue, float NewValue)
{
	OnHealthChanged.Broadcast(this, OldValue, NewValue);
}

void UMYGasLearnHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMYGasLearnHealthAttributeSet, Health, OldHealth);
	BroadcastHealthChanged(OldHealth.GetCurrentValue(), GetHealth());
}

void UMYGasLearnHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMYGasLearnHealthAttributeSet, MaxHealth, OldMaxHealth);
}
