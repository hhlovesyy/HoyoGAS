#include "Stores/PlayerVitalsUIStore.h"

#include "AbilitySystem/HoyoAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "HoyoGasPlayerState.h"

void UPlayerVitalsUIStore::BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState)
{
	Super::BindToPlayerContext(InPawn, InPlayerState);

	const AHoyoGasPlayerState* HoyoPlayerState = Cast<AHoyoGasPlayerState>(InPlayerState);
	BindAbilitySystem(HoyoPlayerState ? HoyoPlayerState->GetAbilitySystemComponent() : nullptr);
}

void UPlayerVitalsUIStore::UnbindFromPlayerContext()
{
	UnbindAbilitySystem();
	Super::UnbindFromPlayerContext();
	BroadcastStoreChanged();
}

float UPlayerVitalsUIStore::GetHealth() const
{
	return Health;
}

float UPlayerVitalsUIStore::GetMaxHealth() const
{
	return MaxHealth;
}

float UPlayerVitalsUIStore::GetHealthPercent() const
{
	return HealthPercent;
}

const FText& UPlayerVitalsUIStore::GetHealthText() const
{
	return HealthText;
}

float UPlayerVitalsUIStore::GetEnergy() const
{
	return Energy;
}

float UPlayerVitalsUIStore::GetMaxEnergy() const
{
	return MaxEnergy;
}

float UPlayerVitalsUIStore::GetEnergyPercent() const
{
	return EnergyPercent;
}

const FText& UPlayerVitalsUIStore::GetEnergyText() const
{
	return EnergyText;
}

const FText& UPlayerVitalsUIStore::GetEnergyRechargeText() const
{
	return EnergyRechargeText;
}

void UPlayerVitalsUIStore::BindAbilitySystem(UAbilitySystemComponent* InAbilitySystemComponent)
{
	if (AbilitySystemComponent.Get() == InAbilitySystemComponent)
	{
		RefreshFromAbilitySystem();
		return;
	}

	UnbindAbilitySystem();
	AbilitySystemComponent = InAbilitySystemComponent;

	if (AbilitySystemComponent.IsValid())
	{
		UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
		//如果你的游戏是联机游戏，伤害通常是在服务器结算的。服务器修改了 Health 之后，通过网络同步（Replication）给客户端。当客户端的属性发生同步改变时（底层通常走 REPNOTIFY），也会完美触发这个 Delegate。所以你的 UI Store 运行在客户端，依然能实时更新。
		HealthChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UHoyoAttributeSet::GetHealthAttribute())
			.AddUObject(this, &UPlayerVitalsUIStore::HandleHealthChanged);//GetGameplayAttributeValueChangeDelegate 监听的是 CurrentValue。只要 CurrentValue 发生变化，它就会触发。
		MaxHealthChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UHoyoAttributeSet::GetMaxHealthAttribute())
			.AddUObject(this, &UPlayerVitalsUIStore::HandleMaxHealthChanged);
		EnergyChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UHoyoAttributeSet::GetEnergyAttribute())
			.AddUObject(this, &UPlayerVitalsUIStore::HandleEnergyChanged);
		MaxEnergyChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UHoyoAttributeSet::GetMaxEnergyAttribute())
			.AddUObject(this, &UPlayerVitalsUIStore::HandleMaxEnergyChanged);
		EnergyRechargeChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UHoyoAttributeSet::GetEnergyRechargeAttribute())
			.AddUObject(this, &UPlayerVitalsUIStore::HandleEnergyRechargeChanged);
	}

	RefreshFromAbilitySystem();
}

void UPlayerVitalsUIStore::UnbindAbilitySystem()
{
	if (AbilitySystemComponent.IsValid())
	{
		UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();

		if (HealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UHoyoAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
		}

		if (MaxHealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UHoyoAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
		}

		if (EnergyChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UHoyoAttributeSet::GetEnergyAttribute()).Remove(EnergyChangedHandle);
		}

		if (MaxEnergyChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UHoyoAttributeSet::GetMaxEnergyAttribute()).Remove(MaxEnergyChangedHandle);
		}

		if (EnergyRechargeChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UHoyoAttributeSet::GetEnergyRechargeAttribute()).Remove(EnergyRechargeChangedHandle);
		}
	}

	HealthChangedHandle.Reset();
	MaxHealthChangedHandle.Reset();
	EnergyChangedHandle.Reset();
	MaxEnergyChangedHandle.Reset();
	EnergyRechargeChangedHandle.Reset();
	AbilitySystemComponent.Reset();

	Health = 0.0f;
	MaxHealth = 0.0f;
	Energy = 0.0f;
	MaxEnergy = 0.0f;
	EnergyRecharge = 1.0f;
	RebuildDisplayValues();
}

void UPlayerVitalsUIStore::RefreshFromAbilitySystem()
{
	const UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC)
	{
		Health = 0.0f;
		MaxHealth = 0.0f;
		Energy = 0.0f;
		MaxEnergy = 0.0f;
		EnergyRecharge = 1.0f;
		RebuildDisplayValues();
		BroadcastStoreChanged();
		return;
	}

	Health = ASC->GetNumericAttribute(UHoyoAttributeSet::GetHealthAttribute());
	MaxHealth = ASC->GetNumericAttribute(UHoyoAttributeSet::GetMaxHealthAttribute());
	Energy = ASC->GetNumericAttribute(UHoyoAttributeSet::GetEnergyAttribute());
	MaxEnergy = ASC->GetNumericAttribute(UHoyoAttributeSet::GetMaxEnergyAttribute());
	EnergyRecharge = ASC->GetNumericAttribute(UHoyoAttributeSet::GetEnergyRechargeAttribute());

	RebuildDisplayValues();
	BroadcastStoreChanged();
}

void UPlayerVitalsUIStore::RebuildDisplayValues()
{
	HealthPercent = MaxHealth > 0.0f ? FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f) : 0.0f;
	EnergyPercent = MaxEnergy > 0.0f ? FMath::Clamp(Energy / MaxEnergy, 0.0f, 1.0f) : 0.0f;

	HealthText = FText::Format(
		INVTEXT("{0} / {1}"),
		FText::AsNumber(FMath::RoundToInt(Health)),
		FText::AsNumber(FMath::RoundToInt(MaxHealth)));

	EnergyText = FText::Format(
		INVTEXT("{0} / {1}"),
		FText::AsNumber(FMath::RoundToInt(Energy)),
		FText::AsNumber(FMath::RoundToInt(MaxEnergy)));

	EnergyRechargeText = FText::Format(
		INVTEXT("{0}%"),
		FText::AsNumber(FMath::RoundToInt(EnergyRecharge * 100.0f)));
}

void UPlayerVitalsUIStore::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	Health = Data.NewValue; //还可以拿OldValue之类的
	RebuildDisplayValues();
	BroadcastStoreChanged();
}

void UPlayerVitalsUIStore::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	MaxHealth = Data.NewValue;
	RebuildDisplayValues();
	BroadcastStoreChanged();
}

void UPlayerVitalsUIStore::HandleEnergyChanged(const FOnAttributeChangeData& Data)
{
	Energy = Data.NewValue;
	RebuildDisplayValues();
	BroadcastStoreChanged();
}

void UPlayerVitalsUIStore::HandleMaxEnergyChanged(const FOnAttributeChangeData& Data)
{
	MaxEnergy = Data.NewValue;
	RebuildDisplayValues();
	BroadcastStoreChanged();
}

void UPlayerVitalsUIStore::HandleEnergyRechargeChanged(const FOnAttributeChangeData& Data)
{
	EnergyRecharge = Data.NewValue;
	RebuildDisplayValues();
	BroadcastStoreChanged();
}
