#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "HoyoProgressionSettings.generated.h"

class UDataTable;
class UHoyoConditionEvaluator;

USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoEventCounterMapping
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Event Mapping")
	FGameplayTag EventTag;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Event Mapping")
	FGameplayTag CounterTag;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Hoyo Progression"))
class HOYOGAS_API UHoyoProgressionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Save")
	FString SaveSlotName = TEXT("HoyoProgress");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Save")
	int32 SaveUserIndex = 0;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Save", meta = (ClampMin = "0.1"))
	float AutoSaveIntervalSeconds = 5.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Achievement")
	TSoftObjectPtr<UDataTable> AchievementDefinitionTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Event Mapping")
	TArray<FHoyoEventCounterMapping> EventCounterMappings;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Condition Evaluation")
	TArray<TSubclassOf<UHoyoConditionEvaluator>> ConditionEvaluatorClasses;
};
