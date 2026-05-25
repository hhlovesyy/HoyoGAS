// Copyright Epic Games, Inc. All Rights Reserved.

#include "HoyoGasPlayerState.h"

#include "AbilitySystem/HoyoAbilitySystemComponent.h"
#include "AbilitySystem/HoyoAttributeSet.h"
#include "Components/CharacterEquipmentComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/ScoreProgressionComponent.h"
#include "Progression/HoyoProgressEventBridgeComponent.h"

AHoyoGasPlayerState::AHoyoGasPlayerState()
{
	SetNetUpdateFrequency(100.0f);

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	ScoreProgressionComponent = CreateDefaultSubobject<UScoreProgressionComponent>(TEXT("ScoreProgressionComponent"));
	CharacterEquipmentComponent = CreateDefaultSubobject<UCharacterEquipmentComponent>(TEXT("CharacterEquipmentComponent"));
	ProgressEventBridgeComponent = CreateDefaultSubobject<UHoyoProgressEventBridgeComponent>(TEXT("ProgressEventBridgeComponent"));

	AbilitySystemComponent = CreateDefaultSubobject<UHoyoAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UHoyoAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AHoyoGasPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
