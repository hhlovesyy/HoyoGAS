#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SurvivorArenaSettings.generated.h"

class UDataTable;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Survivor Arena"))
class HOYOGAS_API USurvivorArenaSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	USurvivorArenaSettings();

	virtual FName GetCategoryName() const override;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Maps")
	FName SurvivorArenaMapName = TEXT("L_SurvivorArena");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> CharacterDefinitionTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> RewardDefinitionTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> EnemyDefinitionTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> WaveDefinitionTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Defaults")
	FName DefaultCharacterId = TEXT("Default");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Defaults")
	FName DefaultLevelId = TEXT("Level_001");
};
