#include "Economy/SurvivorRunEconomyComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Core/SurvivorArenaLog.h"
#include "GAS/SurvivorAttributeSet.h"
#include "Net/UnrealNetwork.h"

USurvivorRunEconomyComponent::USurvivorRunEconomyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void USurvivorRunEconomyComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		BindAbilitySystem(AbilitySystemOwner->GetAbilitySystemComponent());
	}

	EconomyChangedEvent.Broadcast(this);
}

void USurvivorRunEconomyComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USurvivorRunEconomyComponent, Gold);
}

int32 USurvivorRunEconomyComponent::GetGold() const
{
	return Gold;
}

float USurvivorRunEconomyComponent::GetExperience() const
{
	return AbilitySystemComponent.IsValid()
		? AbilitySystemComponent->GetNumericAttribute(USurvivorAttributeSet::GetExperienceAttribute())
		: 0.0f;
}

void USurvivorRunEconomyComponent::AddGold(int32 DeltaGold)
{
	if (DeltaGold == 0)
	{
		return;
	}

	Gold = FMath::Max(0, Gold + DeltaGold);
	UE_LOG(LogSurvivorArena, Log, TEXT("RunEconomy gold changed. Owner=%s Gold=%d Delta=%d"), *GetNameSafe(GetOwner()), Gold, DeltaGold);
	EconomyChangedEvent.Broadcast(this);
}

bool USurvivorRunEconomyComponent::SpendGold(int32 DeltaGold)
{
	if (DeltaGold <= 0)
	{
		return true;
	}

	if (Gold < DeltaGold)
	{
		return false;
	}

	AddGold(-DeltaGold);
	return true;
}

void USurvivorRunEconomyComponent::AddExperience(float DeltaExperience)
{
	if (!AbilitySystemComponent.IsValid() || FMath::IsNearlyZero(DeltaExperience))
	{
		return;
	}

	AbilitySystemComponent->ApplyModToAttributeUnsafe(
		USurvivorAttributeSet::GetExperienceAttribute(),
		EGameplayModOp::Additive,
		DeltaExperience);

	UE_LOG(LogSurvivorArena, Log, TEXT("RunEconomy experience requested. Owner=%s Delta=%.2f"), *GetNameSafe(GetOwner()), DeltaExperience);
}

FOnSurvivorEconomyChanged& USurvivorRunEconomyComponent::OnEconomyChanged()
{
	return EconomyChangedEvent;
}

void USurvivorRunEconomyComponent::BindAbilitySystem(UAbilitySystemComponent* InAbilitySystemComponent)
{
	if (AbilitySystemComponent.Get() == InAbilitySystemComponent)
	{
		return;
	}

	if (AbilitySystemComponent.IsValid() && ExperienceChangedHandle.IsValid())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(USurvivorAttributeSet::GetExperienceAttribute()).Remove(ExperienceChangedHandle);
		ExperienceChangedHandle.Reset();
	}

	AbilitySystemComponent = InAbilitySystemComponent;
	if (AbilitySystemComponent.IsValid())
	{
		ExperienceChangedHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(USurvivorAttributeSet::GetExperienceAttribute())
			.AddUObject(this, &USurvivorRunEconomyComponent::HandleExperienceChanged);
	}
}

void USurvivorRunEconomyComponent::HandleExperienceChanged(const FOnAttributeChangeData& ChangeData)
{
	UE_LOG(LogSurvivorArena, Verbose, TEXT("RunEconomy experience changed. Owner=%s Old=%.2f New=%.2f"), *GetNameSafe(GetOwner()), ChangeData.OldValue, ChangeData.NewValue);
	EconomyChangedEvent.Broadcast(this);
}

void USurvivorRunEconomyComponent::OnRep_Gold()
{
	EconomyChangedEvent.Broadcast(this);
}
