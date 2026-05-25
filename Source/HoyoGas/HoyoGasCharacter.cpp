// Copyright Epic Games, Inc. All Rights Reserved.

#include "HoyoGasCharacter.h"

#include "AbilitySystemComponent.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HoyoGasPlayerState.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AHoyoGasCharacter

AHoyoGasCharacter::AHoyoGasCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = TargetCameraDistance;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	HeadAttachmentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadAttachmentMesh"));
	HeadAttachmentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

USkeletalMeshComponent* AHoyoGasCharacter::ResolveCharacterMesh() const
{
	TArray<USkeletalMeshComponent*> AllMeshes;
	GetComponents<USkeletalMeshComponent>(AllMeshes);

	for (USkeletalMeshComponent* MeshComp : AllMeshes)
	{
		if (MeshComp && MeshComp->GetName() == TEXT("GenshinMesh"))
		{
			return MeshComp;
		}
	}

	return Super::ResolveCharacterMesh();
}

void AHoyoGasCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	// 这个函数是针对server的
	InitAbilityActorInfo();
}

void AHoyoGasCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	// 这个函数是针对client的，需要在同步playerState之后才好设置
	InitAbilityActorInfo();
}

void AHoyoGasCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HeadAttachmentMesh && CharacterMesh)
	{
		HeadAttachmentMesh->AttachToComponent(
			CharacterMesh,
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			TEXT("HeadDecalSocket"));
	}
}

void AHoyoGasCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!CameraBoom)
	{
		return;
	}

	CameraBoom->TargetArmLength = FMath::FInterpTo(
		CameraBoom->TargetArmLength,
		TargetCameraDistance,
		DeltaSeconds,
		ZoomInterpSpeed);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AHoyoGasCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AHoyoGasCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHoyoGasCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHoyoGasCharacter::Look);
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AHoyoGasCharacter::Zoom);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AHoyoGasCharacter::InitAbilityActorInfo()
{
	AHoyoGasPlayerState* HoyoPlayerState = GetPlayerState<AHoyoGasPlayerState>();
	check(HoyoPlayerState);
	HoyoPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(HoyoPlayerState, this);
	AbilitySystemComponent = HoyoPlayerState->GetAbilitySystemComponent();
	AttributeSet = HoyoPlayerState->GetAttributeSet();
}

void AHoyoGasCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AHoyoGasCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X * MouseYawSensitivity);

		const FRotator ControlRotation = Controller->GetControlRotation();
		const float CurrentPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);
		const float NewPitch = FMath::Clamp(
			CurrentPitch + LookAxisVector.Y * MousePitchSensitivity,
			MinPitch,
			MaxPitch);

		Controller->SetControlRotation(FRotator(NewPitch, ControlRotation.Yaw, ControlRotation.Roll));
	}
}

void AHoyoGasCharacter::Zoom(const FInputActionValue& Value)
{
	const float ZoomAxis = Value.Get<float>();
	if (FMath::IsNearlyZero(ZoomAxis))
	{
		return;
	}

	TargetCameraDistance = FMath::Clamp(
		TargetCameraDistance - ZoomAxis * ZoomStep,
		MinCameraDistance,
		MaxCameraDistance);
}
