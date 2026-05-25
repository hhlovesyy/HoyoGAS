#include "GameplayDemoSettings.h"

UGameplayDemoSettings::UGameplayDemoSettings()
{
	DefaultSpawnItemIds = {TEXT("RedGem"), TEXT("BlueOrb"), TEXT("GreenToken")};
}

FName UGameplayDemoSettings::GetCategoryName() const
{
	return TEXT("Game");
}
