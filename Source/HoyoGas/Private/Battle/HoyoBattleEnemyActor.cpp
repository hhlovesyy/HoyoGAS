// Copyright Epic Games, Inc. All Rights Reserved.

#include "Battle/HoyoBattleEnemyActor.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Subsystems/HoyoBattleFlowSubsystem.h"
#include "UObject/ConstructorHelpers.h"

AHoyoBattleEnemyActor::AHoyoBattleEnemyActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(Root);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SkeletalMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	FallbackMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FallbackMeshComponent"));
	FallbackMeshComponent->SetupAttachment(Root);
	FallbackMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FallbackMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	FallbackMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	FallbackMeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		FallbackMeshComponent->SetStaticMesh(CubeMeshFinder.Object);
	}
}

void AHoyoBattleEnemyActor::BeginPlay()
{
	Super::BeginPlay();
	RefreshVisualState();

	if (UHoyoBattleFlowSubsystem* BattleFlowSubsystem = ResolveBattleFlowSubsystem())
	{
		BattleFlowSubsystem->RegisterBattleEnemy(this);
	}
}

void AHoyoBattleEnemyActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UHoyoBattleFlowSubsystem* BattleFlowSubsystem = ResolveBattleFlowSubsystem())
	{
		BattleFlowSubsystem->UnregisterBattleEnemy(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AHoyoBattleEnemyActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshVisualState();
}

void AHoyoBattleEnemyActor::InitializeFromEnemyEntry(const FHoyoBattleEnemyEntry& EnemyEntry)
{
	if (EnemyEntry.EnemyId != NAME_None)
	{
		EnemyId = EnemyEntry.EnemyId;
	}

	if (EnemyEntry.EnemyLevel > 0)
	{
		EnemyLevel = EnemyEntry.EnemyLevel;
	}

	if (!EnemyEntry.DisplayName.IsEmpty())
	{
		DisplayName = EnemyEntry.DisplayName;
	}
}

bool AHoyoBattleEnemyActor::CanBeTargeted_Implementation() const
{
	return bIsTargetable && bIsAlive;
}

FVector AHoyoBattleEnemyActor::GetTargetLocation_Implementation() const
{
	if (SkeletalMeshComponent && SkeletalMeshComponent->IsVisible() && TargetSocketName != NAME_None && SkeletalMeshComponent->DoesSocketExist(TargetSocketName))
	{
		return SkeletalMeshComponent->GetSocketLocation(TargetSocketName);
	}

	return GetActorLocation() + FVector(0.0f, 0.0f, TargetLocationZOffset);
}

void AHoyoBattleEnemyActor::SetEnemyHighlighted_Implementation(bool bHighlighted)
{
	if (bIsHighlighted == bHighlighted)
	{
		return;
	}

	bIsHighlighted = bHighlighted;
	ApplyHighlightToComponent(SkeletalMeshComponent, bHighlighted);
	ApplyHighlightToComponent(FallbackMeshComponent, bHighlighted);
}

bool AHoyoBattleEnemyActor::IsEnemyAlive_Implementation() const
{
	return bIsAlive;
}

FName AHoyoBattleEnemyActor::GetEnemyId_Implementation() const
{
	return EnemyId;
}

FText AHoyoBattleEnemyActor::GetEnemyDisplayName_Implementation() const
{
	return !DisplayName.IsEmpty() ? DisplayName : FText::FromName(EnemyId);
}

int32 AHoyoBattleEnemyActor::GetEnemyLevel_Implementation() const
{
	return EnemyLevel;
}

void AHoyoBattleEnemyActor::RefreshVisualState()
{
	const bool bHasSkeletalMesh = SkeletalMeshComponent && SkeletalMeshComponent->GetSkeletalMeshAsset() != nullptr;

	if (SkeletalMeshComponent)
	{
		SkeletalMeshComponent->SetVisibility(bHasSkeletalMesh, true);
		SkeletalMeshComponent->SetHiddenInGame(!bHasSkeletalMesh, true);
	}

	if (FallbackMeshComponent)
	{
		const bool bUseFallbackMesh = !bHasSkeletalMesh;
		FallbackMeshComponent->SetVisibility(bUseFallbackMesh, true);
		FallbackMeshComponent->SetHiddenInGame(!bUseFallbackMesh, true);
		FallbackMeshComponent->SetWorldScale3D(FVector(1.0f, 1.0f, 1.8f));

		if (UMaterialInstanceDynamic* DynamicMaterial = FallbackMeshComponent->CreateAndSetMaterialInstanceDynamic(0))
		{
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FallbackMeshColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), FallbackMeshColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("TintColor"), FallbackMeshColor);
		}
	}

	ApplyHighlightToComponent(SkeletalMeshComponent, bIsHighlighted);
	ApplyHighlightToComponent(FallbackMeshComponent, bIsHighlighted);
}

void AHoyoBattleEnemyActor::ApplyHighlightToComponent(UPrimitiveComponent* PrimitiveComponent, bool bHighlighted) const
{
	if (!PrimitiveComponent)
	{
		return;
	}

	PrimitiveComponent->SetRenderCustomDepth(bHighlighted);
	PrimitiveComponent->SetCustomDepthStencilValue(HighlightStencilValue);
}

UHoyoBattleFlowSubsystem* AHoyoBattleEnemyActor::ResolveBattleFlowSubsystem() const
{
	const APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	const ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	return LocalPlayer ? LocalPlayer->GetSubsystem<UHoyoBattleFlowSubsystem>() : nullptr;
}
