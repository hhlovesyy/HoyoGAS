#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "Progression/HoyoCheckTypes.h"
#include "HoyoProgressSaveGame.generated.h"

USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoProgressCounterSaveEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FGameplayTag CounterTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 Value = 0;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoProgressValueSaveEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FGameplayTag ValueTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FHoyoProgressValue Value;
};

UCLASS()
class HOYOGAS_API UHoyoProgressSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Legacy save format. Loaded into Values for compatibility.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	TArray<FHoyoProgressCounterSaveEntry> Counters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	TArray<FHoyoProgressValueSaveEntry> Values;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	TArray<FName> UnlockedAchievementIds;
};
