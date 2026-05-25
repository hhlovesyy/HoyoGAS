// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/HoyoEnemy.h"

#include "AbilitySystem/HoyoAbilitySystemComponent.h"
#include "AbilitySystem/HoyoAttributeSet.h"
#include "Components/SkeletalMeshComponent.h"

AHoyoEnemy::AHoyoEnemy()
{
	PrimaryActorTick.bCanEverTick = false;
	AbilitySystemComponent = CreateDefaultSubobject<UHoyoAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<UHoyoAttributeSet>("AttributeSet");
}

void AHoyoEnemy::BeginPlay()
{
	Super::BeginPlay();
	ApplyHighlightState(false);
	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this,this);
}

bool AHoyoEnemy::CanBeTargeted_Implementation() const
{
	return bIsTargetable && bIsAlive;
}

FVector AHoyoEnemy::GetTargetLocation_Implementation() const
{
	if (CharacterMesh && TargetSocketName != NAME_None && CharacterMesh->DoesSocketExist(TargetSocketName))
	{
		return CharacterMesh->GetSocketLocation(TargetSocketName);
	}

	return GetActorLocation() + FVector(0.0f, 0.0f, TargetLocationZOffset);
}

void AHoyoEnemy::SetEnemyHighlighted_Implementation(bool bHighlighted)
{
	if (bIsHighlighted == bHighlighted)
	{
		return;
	}

	bIsHighlighted = bHighlighted;
	ApplyHighlightState(bHighlighted);
}

bool AHoyoEnemy::IsEnemyAlive_Implementation() const
{
	return bIsAlive;
}

FName AHoyoEnemy::GetEnemyId_Implementation() const
{
	return EnemyId;
}

FText AHoyoEnemy::GetEnemyDisplayName_Implementation() const
{
	return !DisplayName.IsEmpty() ? DisplayName : FText::FromName(EnemyId);
}

int32 AHoyoEnemy::GetEnemyLevel_Implementation() const
{
	return EnemyLevel;
}

void AHoyoEnemy::ApplyHighlightState(bool bHighlighted)
{
	if (CharacterMesh)
	{
		CharacterMesh->SetRenderCustomDepth(bHighlighted);
		CharacterMesh->SetCustomDepthStencilValue(HighlightStencilValue);
	}

	if (bHighlightWeapon && Weapon)
	{
		Weapon->SetRenderCustomDepth(bHighlighted);
		Weapon->SetCustomDepthStencilValue(HighlightStencilValue);
	}
}

