#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "OrigamiBirdSettings.generated.h"

class UDataTable;

// Origami Bird Match project settings.
// Centralizes data tables used by the activity, level system, and debug entry points.
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Origami Bird Match"))
class HOYOGAS_API UOrigamiBirdSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;

	// Tile definition table. Each row defines one board tile type.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> TileDefinitionTable;

	// Level definition table. Each row defines one playable level.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> LevelDefinitionTable;

	// Prop definition table. Each row defines one usable prop.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> PropDefinitionTable;

	// Development default level used by quick launch or debug UI.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Development")
	FName DefaultDevelopmentLevelId = TEXT("Level_001");
};
