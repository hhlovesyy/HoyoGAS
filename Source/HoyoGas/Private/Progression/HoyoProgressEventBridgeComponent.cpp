// Fill out your copyright notice in the Description page of Project Settings.


#include "Progression/HoyoProgressEventBridgeComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/HoyoAttributeSet.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Progression/HoyoProgressionTags.h"
#include "Subsystems/MyGameplayTagEventBusSubsystem.h"


UHoyoProgressEventBridgeComponent::UHoyoProgressEventBridgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UHoyoProgressEventBridgeComponent::BeginPlay()
{
	Super::BeginPlay();
	BindAbilitySystem();
}

void UHoyoProgressEventBridgeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindAbilitySystem();
	Super::EndPlay(EndPlayReason);
}

void UHoyoProgressEventBridgeComponent::BindAbilitySystem()
{
	UnbindAbilitySystem();
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;
	IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(OwnerActor); //在虚幻引擎中，UActorComponent（及其派生类）可以被添加到任何 AActor 子类中，而 APlayerState 本身就继承自 AActor
	if (!AbilitySystemOwner) return;
	
	UAbilitySystemComponent* ASC = AbilitySystemOwner->GetAbilitySystemComponent();
	if (!ASC) return;
	AbilitySystemComponent = ASC;
	HealthChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UHoyoAttributeSet::GetHealthAttribute()).AddUObject(this, &UHoyoProgressEventBridgeComponent::HandleHealthChanged);
}

void UHoyoProgressEventBridgeComponent::UnbindAbilitySystem()
{
	if (AbilitySystemComponent.IsValid() && HealthChangedHandle.IsValid())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHoyoAttributeSet::GetHealthAttribute())
			.Remove(HealthChangedHandle);
	}
	HealthChangedHandle.Reset();
	AbilitySystemComponent.Reset();
}

void UHoyoProgressEventBridgeComponent::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue < Data.OldValue)
	{
		//触发扣血相关事件
		PublishProgressEvent(HoyoProgressionTags::Gameplay_Event_Player_HealthDecreased);
	}
}

void UHoyoProgressEventBridgeComponent::PublishProgressEvent(FGameplayTag EventTag) const
{
	if (!EventTag.IsValid()) return;
	const UWorld* World = GetWorld();
	UGameInstance* GameInstance = World? World->GetGameInstance() : nullptr;
	if (!GameInstance) return;
	if (UMyGameplayTagEventBusSubsystem* EventBus = GameInstance->GetSubsystem<UMyGameplayTagEventBusSubsystem>())
	{
		EventBus->Publish(EventTag);
	}
	
}


