#include "Pickups/SurvivorPickupActor.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/SurvivorArenaLog.h"
#include "Data/SurvivorArenaTypes.h"
#include "Economy/SurvivorRunEconomyComponent.h"
#include "Player/SurvivorPlayerState.h"
#include "Pickups/SurvivorPickupDefinition.h"
#include "Player/SurvivorCharacter.h"
#include "Pooling/SurvivorActorPoolSubsystem.h"

ASurvivorPickupActor::ASurvivorPickupActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(SceneRoot);
	CollisionComponent->SetSphereRadius(40.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASurvivorPickupActor::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASurvivorPickupActor::HandlePickupOverlap);
}

void ASurvivorPickupActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!MagnetTargetActor.IsValid() || !PickupDefinition || !PickupDefinition->bAffectedByPickupRadius || bCollected)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = MagnetTargetActor->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	const FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaSeconds, PickupDefinition->MagnetSpeed);
	SetActorLocation(NewLocation);

	if (FVector::DistSquared(NewLocation, TargetLocation) <= FMath::Square(60.0f))
	{
		TryCollect(MagnetTargetActor.Get());
	}
}

void ASurvivorPickupActor::InitializePickup(USurvivorPickupDefinition* InPickupDefinition, int32 InGrantedValue)
{
	PickupDefinition = InPickupDefinition;
	GrantedValue = FMath::Max(0, InGrantedValue);
	bCollected = false;
	MagnetTargetActor.Reset();

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
}

void ASurvivorPickupActor::BeginMagnetTowards(AActor* CollectorActor)
{
	if (!PickupDefinition || !PickupDefinition->bAffectedByPickupRadius || bCollected)
	{
		return;
	}

	MagnetTargetActor = CollectorActor;
}

bool ASurvivorPickupActor::TryCollect(AActor* CollectorActor)
{
	if (bCollected || !PickupDefinition || !CollectorActor)
	{
		return false;
	}

	USurvivorRunEconomyComponent* EconomyComponent = ResolveEconomyComponent(CollectorActor);
	if (!EconomyComponent)
	{
		return false;
	}

	bCollected = true;
	switch (PickupDefinition->PickupType)
	{
	case ESurvivorPickupType::Coin:
		EconomyComponent->AddGold(GrantedValue);
		break;
	case ESurvivorPickupType::Experience:
		EconomyComponent->AddExperience(static_cast<float>(GrantedValue));
		break;
	default:
		break;
	}

	UE_LOG(LogSurvivorArena, Log, TEXT("Pickup collected. Pickup=%s Type=%d Value=%d Collector=%s"),
		*GetNameSafe(this),
		static_cast<int32>(PickupDefinition->PickupType),
		GrantedValue,
		*GetNameSafe(CollectorActor));

	ReleaseToPoolOrDestroy();
	return true;
}

void ASurvivorPickupActor::OnAcquiredFromPool_Implementation()
{
	bWasAcquiredFromPool = true;
	bCollected = false;
	MagnetTargetActor.Reset();
}

void ASurvivorPickupActor::OnReleasedToPool_Implementation()
{
	PickupDefinition = nullptr;
	GrantedValue = 0;
	bCollected = false;
	MagnetTargetActor.Reset();
}

void ASurvivorPickupActor::HandlePickupOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (PickupDefinition && PickupDefinition->bAutoPickupOnTouch)
	{
		TryCollect(OtherActor);
	}
}

USurvivorRunEconomyComponent* ASurvivorPickupActor::ResolveEconomyComponent(AActor* CollectorActor) const
{
	if (ASurvivorCharacter* SurvivorCharacter = Cast<ASurvivorCharacter>(CollectorActor))
	{
		if (ASurvivorPlayerState* SurvivorPlayerState = SurvivorCharacter->GetPlayerState<ASurvivorPlayerState>())
		{
			return SurvivorPlayerState->GetRunEconomyComponent();
		}
	}

	if (ASurvivorPlayerState* SurvivorPlayerState = Cast<ASurvivorPlayerState>(CollectorActor))
	{
		return SurvivorPlayerState->GetRunEconomyComponent();
	}

	return nullptr;
}

void ASurvivorPickupActor::ReleaseToPoolOrDestroy()
{
	if (USurvivorActorPoolSubsystem* PoolSubsystem = GetWorld() ? GetWorld()->GetSubsystem<USurvivorActorPoolSubsystem>() : nullptr)
	{
		PoolSubsystem->ReleaseActor(this);
		return;
	}

	Destroy();
}
