#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SurvivorWeaponDefinition.generated.h"

class USurvivorFirePattern;

UCLASS(BlueprintType)
class HOYOGAS_API USurvivorWeaponDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName WeaponId = TEXT("DebugWeapon");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0.01"))
	float FireInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Weapon")
	TObjectPtr<USurvivorFirePattern> FirePattern = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug", meta = (ClampMin = "0.0"))
	float DebugLineLength = 1000.0f;
};
