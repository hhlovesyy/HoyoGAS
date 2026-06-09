#include "Player/SurvivorCharacter.h"

#include "Core/SurvivorArenaLog.h"
#include "FaceShadowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GAS/SurvivorAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/SurvivorPlayerState.h"
#include "Pickups/SurvivorPickupCollectorComponent.h"
#include "Weapons/SurvivorWeaponManagerComponent.h"

ASurvivorCharacter::ASurvivorCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement())
	{
		CharacterMovementComponent->bOrientRotationToMovement = true;
		CharacterMovementComponent->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	}

	CameraPivot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraPivot"));
	CameraPivot->SetupAttachment(RootComponent);
	CameraPivot->SetRelativeLocation(CameraPivotRelativeLocation);

	GetMesh()->SetHiddenInGame(true);
	GetMesh()->SetVisibility(false, true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCastShadow(false);
	GetMesh()->bReceivesDecals = false;
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	VisualCharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VisualCharacterMesh"));
	VisualCharacterMesh->SetupAttachment(GetMesh());
	VisualCharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualCharacterMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	HeadAttachmentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadAttachmentMesh"));
	HeadAttachmentMesh->SetupAttachment(VisualCharacterMesh);
	HeadAttachmentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(VisualCharacterMesh);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OutlineMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("OutlineMesh"));
	OutlineMesh->SetupAttachment(VisualCharacterMesh);
	OutlineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OutlineMesh->SetCastShadow(false);
	OutlineMesh->bReceivesDecals = false;
	OutlineMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	FaceShadowComponent = CreateDefaultSubobject<UFaceShadowComponent>(TEXT("FaceShadowComponent"));
	FaceShadowComponent->SetCharacterMeshComponent(VisualCharacterMesh);

	WeaponManagerComponent = CreateDefaultSubobject<USurvivorWeaponManagerComponent>(TEXT("WeaponManagerComponent"));
	PickupCollectorComponent = CreateDefaultSubobject<USurvivorPickupCollectorComponent>(TEXT("PickupCollectorComponent"));
}

void ASurvivorCharacter::BeginPlay()
{
	Super::BeginPlay();
	RefreshVisualComponentBindings();
}

void ASurvivorCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
}

void ASurvivorCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
}

UAbilitySystemComponent* ASurvivorCharacter::GetAbilitySystemComponent() const
{
	return CachedAbilitySystemComponent;
}

USkeletalMeshComponent* ASurvivorCharacter::GetSourceCharacterMesh() const
{
	return GetMesh();
}

USkeletalMeshComponent* ASurvivorCharacter::GetVisualCharacterMesh() const
{
	return VisualCharacterMesh;
}

USkeletalMeshComponent* ASurvivorCharacter::GetWeaponMesh() const
{
	return WeaponMesh;
}

USkeletalMeshComponent* ASurvivorCharacter::GetOutlineMesh() const
{
	return OutlineMesh;
}

UStaticMeshComponent* ASurvivorCharacter::GetHeadAttachmentMesh() const
{
	return HeadAttachmentMesh;
}

UFaceShadowComponent* ASurvivorCharacter::GetFaceShadowComponent() const
{
	return FaceShadowComponent;
}

USceneComponent* ASurvivorCharacter::GetCameraPivot() const
{
	return CameraPivot;
}

FVector ASurvivorCharacter::GetCameraPivotLocation() const
{
	return CameraPivot ? CameraPivot->GetComponentLocation() : GetActorLocation() + CameraPivotRelativeLocation;
}

USurvivorWeaponManagerComponent* ASurvivorCharacter::GetWeaponManagerComponent() const
{
	return WeaponManagerComponent;
}

USurvivorPickupCollectorComponent* ASurvivorCharacter::GetPickupCollectorComponent() const
{
	return PickupCollectorComponent;
}

USkeletalMeshComponent* ASurvivorCharacter::ResolvePrimaryVisualMesh() const
{
	return VisualCharacterMesh.Get() ? VisualCharacterMesh.Get() : GetMesh();
}

void ASurvivorCharacter::InitAbilityActorInfo()
{
	ASurvivorPlayerState* SurvivorPlayerState = GetPlayerState<ASurvivorPlayerState>();
	if (!SurvivorPlayerState)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("SurvivorCharacter %s could not initialize ASC because PlayerState is null."), *GetNameSafe(this));
		CachedAbilitySystemComponent = nullptr;
		return;
	}

	CachedAbilitySystemComponent = SurvivorPlayerState->GetSurvivorAbilitySystemComponent();
	if (!CachedAbilitySystemComponent)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("SurvivorCharacter %s could not initialize ASC because ASC is null."), *GetNameSafe(this));
		return;
	}

	CachedAbilitySystemComponent->InitAbilityActorInfo(SurvivorPlayerState, this);
	UE_LOG(LogSurvivorArena, Log, TEXT("SurvivorCharacter initialized ASC ActorInfo. Character=%s PlayerState=%s"), *GetNameSafe(this), *GetNameSafe(SurvivorPlayerState));
}

void ASurvivorCharacter::RefreshVisualComponentBindings()
{
	USkeletalMeshComponent* PrimaryVisualMesh = ResolvePrimaryVisualMesh();

	if (VisualCharacterMesh)
	{
		VisualCharacterMesh->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::KeepRelativeTransform);
	}

	if (CameraPivot)
	{
		CameraPivot->SetRelativeLocation(CameraPivotRelativeLocation);
	}

	if (HeadAttachmentMesh && PrimaryVisualMesh)
	{
		HeadAttachmentMesh->AttachToComponent(
			PrimaryVisualMesh,
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			HeadAttachmentSocketName);
	}

	if (WeaponMesh && PrimaryVisualMesh)
	{
		WeaponMesh->AttachToComponent(
			PrimaryVisualMesh,
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			WeaponAttachSocketName);
	}

	if (OutlineMesh && PrimaryVisualMesh)
	{
		OutlineMesh->AttachToComponent(
			PrimaryVisualMesh,
			FAttachmentTransformRules::SnapToTargetIncludingScale);
		OutlineMesh->SetLeaderPoseComponent(PrimaryVisualMesh);
	}

	if (FaceShadowComponent && PrimaryVisualMesh)
	{
		FaceShadowComponent->SetCharacterMeshComponent(PrimaryVisualMesh);
	}
}
