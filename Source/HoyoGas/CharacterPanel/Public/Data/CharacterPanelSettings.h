#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CharacterPanelSettings.generated.h"

class UDataTable;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Character Panel"))
class HOYOGAS_API UCharacterPanelSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UCharacterPanelSettings();

	virtual FName GetCategoryName() const override;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> CharacterDefinitionTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	FName DefaultCharacterId = TEXT("Trailblazer_Stelle");
};
