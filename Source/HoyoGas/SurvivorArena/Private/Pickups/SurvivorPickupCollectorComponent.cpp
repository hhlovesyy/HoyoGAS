#include "Pickups/SurvivorPickupCollectorComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Engine/OverlapResult.h"
#include "Core/SurvivorArenaLog.h"
#include "GAS/SurvivorAttributeSet.h"
#include "Player/SurvivorCharacter.h"
#include "Pickups/SurvivorPickupActor.h"

USurvivorPickupCollectorComponent::USurvivorPickupCollectorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USurvivorPickupCollectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastScan += DeltaTime;
	if (TimeSinceLastScan < ScanInterval)
	{
		return;
	}

	TimeSinceLastScan = 0.0f;
	ScanForNearbyPickups();
}

void USurvivorPickupCollectorComponent::ScanForNearbyPickups()
{
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (!World || !OwnerActor)
	{
		return;
	}

	const FVector Origin = OwnerActor->GetActorLocation();
	const float Radius = ResolvePickupRadius() + ScanRadiusPadding;
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	const bool bAnyOverlap = World->OverlapMultiByObjectType(
		Overlaps,
		Origin,
		FQuat::Identity,
		QueryParams,
		FCollisionShape::MakeSphere(Radius));

	if (!bAnyOverlap)
	{
		return;
	}

	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (ASurvivorPickupActor* PickupActor = Cast<ASurvivorPickupActor>(Overlap.GetActor()))
		{
			PickupActor->BeginMagnetTowards(OwnerActor);
		}
	}
}

float USurvivorPickupCollectorComponent::ResolvePickupRadius() const
{
	if (const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (const UAbilitySystemComponent* ASC = AbilitySystemOwner->GetAbilitySystemComponent())
		{
			return ASC->GetNumericAttribute(USurvivorAttributeSet::GetPickupRadiusAttribute());
		}
	}

	return 0.0f;
}
