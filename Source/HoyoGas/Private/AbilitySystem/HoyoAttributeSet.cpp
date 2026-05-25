// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/HoyoAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UHoyoAttributeSet::UHoyoAttributeSet()
{
	InitHealth(100.0f); //通常不这么直接使用，而是使用GE来实现（因为是可预测的），后面会有
	InitMaxHealth(150.0f);
	InitAttackPower(100.0f);
	InitMaxEnergy(300.f);
}

void UHoyoAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	NewValue = ClampAttributeValue(Attribute, NewValue);
}

void UHoyoAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		ClampHealthToMaxHealth();
	}
	else if (Attribute == GetMaxEnergyAttribute())
	{
		ClampEnergyToMaxEnergy();
	}
}

void UHoyoAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute& Attribute = Data.EvaluatedData.Attribute;
	if (Attribute == GetHealthAttribute())
	{
		ClampHealthToMaxHealth();
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		SetMaxHealth(FMath::Max(0.0f, GetMaxHealth()));
		ClampHealthToMaxHealth();
	}
	else if (Attribute == GetEnergyAttribute())
	{
		ClampEnergyToMaxEnergy();
	}
	else if (Attribute == GetMaxEnergyAttribute())
	{
		SetMaxEnergy(FMath::Max(0.0f, GetMaxEnergy()));
		ClampEnergyToMaxEnergy();
	}
	else
	{
		float ClampedValue = ClampAttributeValue(Attribute, Attribute.GetNumericValue(this));
		Attribute.SetNumericValueChecked(ClampedValue, this);
	}
}

void UHoyoAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	//当前这个类里，有哪些变量需要通过网络，从服务器（Server）同步到客户端（Client）
	//如果你在头文件里用 UPROPERTY(Replicated) 标记了一个变量，引擎确实知道了“这个变量有网络同步的潜质”，但它依然不会自动同步。
	//你必须在 .cpp 文件中重写（Override） GetLifetimeReplicatedProps 函数，并在里面正式向引擎注册这个变量。只有注册过的变量，服务器才会在它的值发生改变时，自动把新值发送给各个客户端。
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UHoyoAttributeSet, Health, COND_None, REPNOTIFY_Always); //DOREPLIFETIME_CONDITION_NOTIFY(类名, 变量名, 发送条件, 触发条件)
	//REPNOTIFY_OnChanged 在带客户端预测（Client Prediction）的游戏里，这是致命的！比如客户端预测自己吃了个血瓶，血量从 40 变成了 50，UI 也更新了。半秒后，服务器确认了吃血瓶，把正确的 50 发给客户端。如果是默认行为，客户端一看 50==50，就不触发回调了，导致 GAS 内部的状态机无法正常结算预测的结束。
	DOREPLIFETIME_CONDITION_NOTIFY(UHoyoAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHoyoAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHoyoAttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHoyoAttributeSet, ElementalMastery, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHoyoAttributeSet, Level, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHoyoAttributeSet, CritRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHoyoAttributeSet, CritDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHoyoAttributeSet, Energy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHoyoAttributeSet, MaxEnergy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHoyoAttributeSet, EnergyRecharge, COND_None, REPNOTIFY_Always);
}

void UHoyoAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const //这里的OldHealth参数指的是老的值，对于GAS来说是很有用的
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHoyoAttributeSet, Health, OldHealth); //这个宏负责回滚预测、重新计算当前的 Buff（GameplayEffect）聚合结果。如果不强行触发，整个技能系统的网络预测就会彻底崩溃。
}

void UHoyoAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHoyoAttributeSet, MaxHealth, OldMaxHealth);
}

void UHoyoAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHoyoAttributeSet, AttackPower, OldAttackPower);
}

void UHoyoAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldDefense) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHoyoAttributeSet, Defense, OldDefense);
}

void UHoyoAttributeSet::OnRep_ElementalMastery(const FGameplayAttributeData& OldElementalMastery) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHoyoAttributeSet, ElementalMastery, OldElementalMastery);
}

void UHoyoAttributeSet::OnRep_Level(const FGameplayAttributeData& OldLevel) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHoyoAttributeSet, Level, OldLevel);
}

void UHoyoAttributeSet::OnRep_CritRate(const FGameplayAttributeData& OldCritRate) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHoyoAttributeSet, CritRate, OldCritRate);
}

void UHoyoAttributeSet::OnRep_CritDamage(const FGameplayAttributeData& OldCritDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHoyoAttributeSet, CritDamage, OldCritDamage);
}

void UHoyoAttributeSet::OnRep_Energy(const FGameplayAttributeData& OldEnergy) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHoyoAttributeSet, Energy, OldEnergy);
}

void UHoyoAttributeSet::OnRep_MaxEnergy(const FGameplayAttributeData& OldMaxEnergy) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHoyoAttributeSet, MaxEnergy, OldMaxEnergy);
}

void UHoyoAttributeSet::OnRep_EnergyRecharge(const FGameplayAttributeData& OldEnergyRecharge) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHoyoAttributeSet, EnergyRecharge, OldEnergyRecharge);
}

float UHoyoAttributeSet::ClampAttributeValue(const FGameplayAttribute& Attribute, float Value) const
{
	if (Attribute == GetHealthAttribute())
	{
		return FMath::Clamp(Value, 0.0f, GetMaxHealth());
	}

	if (Attribute == GetEnergyAttribute())
	{
		return FMath::Clamp(Value, 0.0f, GetMaxEnergy());
	}

	if (Attribute == GetCritRateAttribute())
	{
		return FMath::Clamp(Value, 0.0f, 1.0f);
	}

	if (Attribute == GetLevelAttribute())
	{
		return FMath::Max(1.0f, Value);
	}

	if (Attribute == GetMaxHealthAttribute() ||
		Attribute == GetAttackPowerAttribute() ||
		Attribute == GetDefenseAttribute() ||
		Attribute == GetElementalMasteryAttribute() ||
		Attribute == GetCritDamageAttribute() ||
		Attribute == GetMaxEnergyAttribute() ||
		Attribute == GetEnergyRechargeAttribute())
	{
		return FMath::Max(0.0f, Value);
	}

	return Value;
}

void UHoyoAttributeSet::ClampHealthToMaxHealth()
{
	SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
}

void UHoyoAttributeSet::ClampEnergyToMaxEnergy()
{
	SetEnergy(FMath::Clamp(GetEnergy(), 0.0f, GetMaxEnergy()));
}
