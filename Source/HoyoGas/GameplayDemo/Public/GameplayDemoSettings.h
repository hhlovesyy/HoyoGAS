#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayDemoSettings.generated.h"

class UDataTable;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Gameplay Demo"))
class HOYOGAS_API UGameplayDemoSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGameplayDemoSettings();

	virtual FName GetCategoryName() const override;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> ItemDefinitionTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> LevelProgressionTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	bool bAutoSpawnPickups = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "1"))
	int32 DefaultPickupCount = 14;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "100.0"))
	float PickupSpawnRadius = 900.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "0.0"))
	float PickupSpawnHeight = 40.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TArray<FName> DefaultSpawnItemIds;
};
