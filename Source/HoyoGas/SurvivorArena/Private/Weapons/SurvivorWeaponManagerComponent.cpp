#include "Weapons/SurvivorWeaponManagerComponent.h"

#include "Core/SurvivorArenaLog.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Weapons/SurvivorFirePattern.h"
#include "Weapons/SurvivorWeaponDefinition.h"

USurvivorWeaponManagerComponent::USurvivorWeaponManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USurvivorWeaponManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogSurvivorArena, Log, TEXT("WeaponManager BeginPlay. Owner=%s AutoStart=%s WeaponDefinition=%s"),
		*GetNameSafe(GetOwner()),
		bAutoStartDebugWeapon ? TEXT("true") : TEXT("false"),
		*GetNameSafe(DebugWeaponDefinition));

	if (bAutoStartDebugWeapon && DebugWeaponDefinition)
	{
		StartDebugWeapon();
	}
	else if (bAutoStartDebugWeapon && !DebugWeaponDefinition)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("WeaponManager BeginPlay skipped autostart because DebugWeaponDefinition is null. Owner=%s"), *GetNameSafe(GetOwner()));
	}
}

void USurvivorWeaponManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopDebugWeapon();
	Super::EndPlay(EndPlayReason);
}

void USurvivorWeaponManagerComponent::StartDebugWeapon()
{
	if (!DebugWeaponDefinition)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("StartDebugWeapon failed because DebugWeaponDefinition is null. Owner=%s"), *GetNameSafe(GetOwner()));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("StartDebugWeapon failed because World is null. Owner=%s"), *GetNameSafe(GetOwner()));
		return;
	}

	const float FireInterval = FMath::Max(DebugWeaponDefinition->FireInterval, 0.01f);
	World->GetTimerManager().SetTimer(DebugWeaponTimerHandle, this, &USurvivorWeaponManagerComponent::HandleDebugWeaponFire, FireInterval, true);

	UE_LOG(LogSurvivorArena, Log, TEXT("StartDebugWeapon. Owner=%s WeaponId=%s FireInterval=%.3f"),
		*GetNameSafe(GetOwner()),
		*DebugWeaponDefinition->WeaponId.ToString(),
		FireInterval);
}

void USurvivorWeaponManagerComponent::StopDebugWeapon()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DebugWeaponTimerHandle);
	}

	UE_LOG(LogSurvivorArena, Log, TEXT("StopDebugWeapon. Owner=%s"), *GetNameSafe(GetOwner()));
}

void USurvivorWeaponManagerComponent::HandleDebugWeaponFire()
{
	if (!DebugWeaponDefinition)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("HandleDebugWeaponFire skipped because DebugWeaponDefinition is null. Owner=%s"), *GetNameSafe(GetOwner()));
		return;
	}

	if (!DebugWeaponDefinition->FirePattern)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("HandleDebugWeaponFire skipped because FirePattern is null. Owner=%s WeaponId=%s"),
			*GetNameSafe(GetOwner()),
			*DebugWeaponDefinition->WeaponId.ToString());
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("HandleDebugWeaponFire skipped because Owner is null."));
		return;
	}

	FSurvivorFirePatternContext PatternContext;
	PatternContext.OwnerActor = OwnerActor;
	PatternContext.Origin = OwnerActor->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	PatternContext.ForwardDirection = OwnerActor->GetActorForwardVector();
	PatternContext.ForwardDirection.Z = 0.0f;
	if (!PatternContext.ForwardDirection.Normalize())
	{
		PatternContext.ForwardDirection = FVector::ForwardVector;
	}

	TArray<FVector> Directions;
	DebugWeaponDefinition->FirePattern->GenerateDirections(PatternContext, Directions);

	UE_LOG(LogSurvivorArena, Log, TEXT("DebugWeaponFire. Owner=%s WeaponId=%s DirectionCount=%d"),
		*GetNameSafe(OwnerActor),
		*DebugWeaponDefinition->WeaponId.ToString(),
		Directions.Num());

	if (bEnableWeaponDebug)
	{
		DrawDebugFireDirections(Directions, DebugWeaponDefinition->DebugLineLength);
	}
}

void USurvivorWeaponManagerComponent::DrawDebugFireDirections(const TArray<FVector>& Directions, float LineLength)
{
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (!World || !OwnerActor)
	{
		return;
	}

	const FVector LineOrigin = OwnerActor->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	const float SafeLineLength = FMath::Max(LineLength, 0.0f);

	for (FVector Direction : Directions)
	{
		Direction.Z = 0.0f;
		if (!Direction.Normalize())
		{
			continue;
		}

		DrawDebugLine(
			World,
			LineOrigin,
			LineOrigin + (Direction * SafeLineLength),
			FColor::Green,
			false,
			0.45f,
			0,
			2.0f);
	}
}
