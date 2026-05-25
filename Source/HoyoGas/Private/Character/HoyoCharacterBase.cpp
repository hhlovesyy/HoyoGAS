// Fill out your copyright notice in the Description page of Project Settings.


#include "HoyoGas/Public/Character/HoyoCharacterBase.h"

AHoyoCharacterBase::AHoyoCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

UAbilitySystemComponent* AHoyoCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AHoyoCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	CharacterMesh = ResolveCharacterMesh();
	AttachWeaponToCharacterMesh();
}

USkeletalMeshComponent* AHoyoCharacterBase::ResolveCharacterMesh() const
{
	return GetMesh();
}

FName AHoyoCharacterBase::GetWeaponAttachSocketName() const
{
	return TEXT("WeaponHandSocket");
}

void AHoyoCharacterBase::AttachWeaponToCharacterMesh()
{
	if (Weapon && CharacterMesh)
	{
		Weapon->AttachToComponent(
			CharacterMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			GetWeaponAttachSocketName());
	}
}

