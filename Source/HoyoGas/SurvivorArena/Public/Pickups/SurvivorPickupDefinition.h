#pragma once

#include "CoreMinimal.h"
#include "Data/SurvivorArenaTypes.h"
#include "Engine/DataAsset.h"
#include "Pickups/SurvivorPickupActor.h"
#include "SurvivorPickupDefinition.generated.h"

class UTexture2D;

UCLASS(BlueprintType)
class HOYOGAS_API USurvivorPickupDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	FName PickupId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	ESurvivorPickupType PickupType = ESurvivorPickupType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "0"))
	int32 BaseValue = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TSubclassOf<ASurvivorPickupActor> PickupActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	bool bAutoPickupOnTouch = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	bool bAffectedByPickupRadius = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "0.0"))
	float MagnetSpeed = 900.0f;

	bool ValidateDefinition(FString* OutError = nullptr) const;
};
