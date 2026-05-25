// Copyright Epic Games, Inc. All Rights Reserved.

#include "Battle/HoyoBattleTargetMarkerActor.h"

#include "Character/HoyoEnemyInterface.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/PlayerController.h"

AHoyoBattleTargetMarkerActor::AHoyoBattleTargetMarkerActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	TextRenderComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRenderComponent"));
	TextRenderComponent->SetupAttachment(Root);
	TextRenderComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	TextRenderComponent->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	TextRenderComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TextRenderComponent->SetTextRenderColor(MarkerColor);
	TextRenderComponent->SetText(MarkerText);
	TextRenderComponent->SetWorldSize(TextWorldSize);

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void AHoyoBattleTargetMarkerActor::BeginPlay()
{
	Super::BeginPlay();
	OwningPlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	RefreshMarkerVisuals();
}

void AHoyoBattleTargetMarkerActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsTrackingTarget())
	{
		return;
	}

	UpdateTransformFromTrackedTarget();
	FaceOwningPlayerCamera();
}

void AHoyoBattleTargetMarkerActor::SetTrackedTarget(AActor* InTargetActor, const FVector& InWorldLocation)
{
	TargetActor = InTargetActor;
	TrackedWorldLocation = InWorldLocation;
	SetActorHiddenInGame(!IsTrackingTarget());

	if (IsTrackingTarget())
	{
		UpdateTransformFromTrackedTarget();
		FaceOwningPlayerCamera();
	}
}

void AHoyoBattleTargetMarkerActor::ClearTrackedTarget()
{
	TargetActor.Reset();
	SetActorHiddenInGame(true);
}

bool AHoyoBattleTargetMarkerActor::IsTrackingTarget() const
{
	return TargetActor.IsValid();
}

void AHoyoBattleTargetMarkerActor::ConfigureMarker(const FText& InMarkerText, const FColor& InMarkerColor, float InTextWorldSize, float InVerticalOffset)
{
	MarkerText = InMarkerText;
	MarkerColor = InMarkerColor;
	TextWorldSize = InTextWorldSize;
	VerticalOffset = InVerticalOffset;
	RefreshMarkerVisuals();
}

void AHoyoBattleTargetMarkerActor::RefreshMarkerVisuals()
{
	if (!TextRenderComponent)
	{
		return;
	}

	TextRenderComponent->SetText(MarkerText);
	TextRenderComponent->SetTextRenderColor(MarkerColor);
	TextRenderComponent->SetWorldSize(TextWorldSize);
}

void AHoyoBattleTargetMarkerActor::UpdateTransformFromTrackedTarget()
{
	if (AActor* ResolvedTargetActor = TargetActor.Get())
	{
		if (ResolvedTargetActor->GetClass()->ImplementsInterface(UHoyoEnemyInterface::StaticClass()))
		{
			TrackedWorldLocation = IHoyoEnemyInterface::Execute_GetTargetLocation(ResolvedTargetActor);
		}
		else
		{
			TrackedWorldLocation = ResolvedTargetActor->GetActorLocation();
		}
	}

	SetActorLocation(TrackedWorldLocation + FVector(0.0f, 0.0f, VerticalOffset));
}

void AHoyoBattleTargetMarkerActor::FaceOwningPlayerCamera()
{
	APlayerController* PlayerController = OwningPlayerController.Get();
	if (!PlayerController)
	{
		PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
		OwningPlayerController = PlayerController;
	}

	if (!PlayerController || !PlayerController->PlayerCameraManager)
	{
		return;
	}

	const FVector CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	const FRotator FaceCameraRotation = (CameraLocation - GetActorLocation()).Rotation();
	SetActorRotation(FRotator(0.0f, FaceCameraRotation.Yaw + 180.0f, 0.0f));
}
