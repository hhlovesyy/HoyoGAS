#include "GAS/SurvivorAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

USurvivorAttributeSet::USurvivorAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitMoveSpeed(600.0f);
	InitAttackPower(10.0f);
	InitAttackSpeed(1.0f);
	InitPickupRadius(150.0f);
	InitExperience(0.0f);
}

void USurvivorAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USurvivorAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USurvivorAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USurvivorAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USurvivorAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USurvivorAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USurvivorAttributeSet, PickupRadius, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USurvivorAttributeSet, Experience, COND_None, REPNOTIFY_Always);
}

//PreAttributeChange 只管限制将要变成的数值本身；而 PostGameplayEffectExecute 负责处理属性改变后引发的连锁反应（比如血量归零导致死亡）。
void USurvivorAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	NewValue = ClampAttributeValue(Attribute, NewValue);
}

void USurvivorAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute& Attribute = Data.EvaluatedData.Attribute;
	if (Attribute == GetHealthAttribute())
	{
		ClampHealthToMaxHealth();
		return;
	}

	if (Attribute == GetMaxHealthAttribute())
	{
		SetMaxHealth(FMath::Max(0.0f, GetMaxHealth()));
		ClampHealthToMaxHealth();
		return;
	}

	float ClampedValue = ClampAttributeValue(Attribute, Attribute.GetNumericValue(this));
	Attribute.SetNumericValueChecked(ClampedValue, this);
}

void USurvivorAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue) const
{
	//必须调用 GAMEPLAYATTRIBUTE_REPNOTIFY 宏： 当客户端通过网络收到服务器发来的新属性值时，必须调用这个宏。它负责通知 GAS 系统底层“属性已经被网络同步更新了”，这样依赖该属性的 UI (HUD) 才能收到回调并正确刷新（例如血条进度条的变化）。
	GAMEPLAYATTRIBUTE_REPNOTIFY(USurvivorAttributeSet, Health, OldValue);
}

void USurvivorAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USurvivorAttributeSet, MaxHealth, OldValue);
}

void USurvivorAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USurvivorAttributeSet, MoveSpeed, OldValue);
}

void USurvivorAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USurvivorAttributeSet, AttackPower, OldValue);
}

void USurvivorAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USurvivorAttributeSet, AttackSpeed, OldValue);
}

void USurvivorAttributeSet::OnRep_PickupRadius(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USurvivorAttributeSet, PickupRadius, OldValue);
}

void USurvivorAttributeSet::OnRep_Experience(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USurvivorAttributeSet, Experience, OldValue);
}

float USurvivorAttributeSet::ClampAttributeValue(const FGameplayAttribute& Attribute, float Value) const
{
	if (Attribute == GetHealthAttribute())
	{
		return FMath::Clamp(Value, 0.0f, GetMaxHealth());
	}

	if (Attribute == GetMaxHealthAttribute() ||
		Attribute == GetMoveSpeedAttribute() ||
		Attribute == GetAttackPowerAttribute() ||
		Attribute == GetAttackSpeedAttribute() ||
		Attribute == GetPickupRadiusAttribute() ||
		Attribute == GetExperienceAttribute())
	{
		return FMath::Max(0.0f, Value);
	}

	return Value;
}

void USurvivorAttributeSet::ClampHealthToMaxHealth()
{
	SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
}
