#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UIFrameworkTypes.h"
#include "MyUIRegistryDataAsset.generated.h"

UCLASS(BlueprintType)
class HOYOGAS_API UMyUIRegistryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TArray<FMyUIScreenConfig> Screens;
};
