#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LevelProgressionRow.generated.h"

USTRUCT(BlueprintType)
struct HOYOGAS_API FLevelProgressionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo")
	int32 RequiredScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo")
	FText RewardText;
};
