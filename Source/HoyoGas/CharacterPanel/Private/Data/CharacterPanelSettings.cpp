#include "Data/CharacterPanelSettings.h"

UCharacterPanelSettings::UCharacterPanelSettings()
{
	DefaultCharacterId = TEXT("Trailblazer_Stelle");
}

FName UCharacterPanelSettings::GetCategoryName() const
{
	return TEXT("Game");
}
