// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/HoyoEffectActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HoyoAbilitySystemComponent.h"
#include "AbilitySystem/HoyoAttributeSet.h"
#include "Progression/HoyoProgressionTags.h"
#include "Subsystems/MyGameplayTagEventBusSubsystem.h"

AHoyoEffectActor::AHoyoEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}

void AHoyoEffectActor::BeginPlay()
{
	Super::BeginPlay();
}

void AHoyoEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr) return;
	
	check(GameplayEffectClass);
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, 1.0f, EffectContextHandle);
	TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get()); //把一个已经构造好的 GameplayEffectSpec 应用到这个 ASC 自己身上。.Data的类型是TSharedPtr<FGameplayEffectSpec>，通过Get()获得裸指针，再通过*解引用获得const T&
}

void AHoyoEffectActor::BroadcastHoyoEffectPickedUp()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UMyGameplayTagEventBusSubsystem* EventBus = GameInstance->GetSubsystem<UMyGameplayTagEventBusSubsystem>())
		{
			UE_LOG(LogTemp, Display, TEXT("[Progression] HoyoEffectPickedUp broadcast Actor=%s EventTag=%s"),
				*GetNameSafe(this),
				*FGameplayTag(HoyoProgressionTags::Gameplay_Event_HoyoEffectPickedUp).ToString());
			EventBus->Publish(HoyoProgressionTags::Gameplay_Event_HoyoEffectPickedUp);
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[Progression] HoyoEffectPickedUp broadcast failed Actor=%s"), *GetNameSafe(this));
}


