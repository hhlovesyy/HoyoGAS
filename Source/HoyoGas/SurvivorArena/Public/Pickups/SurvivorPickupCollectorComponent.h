#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivorPickupCollectorComponent.generated.h"

UCLASS(ClassGroup = (Survivor), meta = (BlueprintSpawnableComponent))
class HOYOGAS_API USurvivorPickupCollectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USurvivorPickupCollectorComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	void ScanForNearbyPickups();
	float ResolvePickupRadius() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survivor|Pickup", meta = (ClampMin = "0.01"))
	float ScanInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survivor|Pickup", meta = (ClampMin = "0.0"))
	float ScanRadiusPadding = 30.0f;

	float TimeSinceLastScan = 0.0f;
};
