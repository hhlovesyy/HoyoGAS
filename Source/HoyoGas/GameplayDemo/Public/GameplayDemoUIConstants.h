#pragma once

#include "CoreMinimal.h"

struct FGameplayDemoUIConstants
{
	static FName HUDScreenTag()
	{
		static const FName Name(TEXT("GameplayDemoHUD"));
		return Name;
	}

	static FName InventoryScreenTag()
	{
		static const FName Name(TEXT("GameplayDemoInventory"));
		return Name;
	}

	static FName LevelUpDialogTag()
	{
		static const FName Name(TEXT("GameplayDemoLevelUp"));
		return Name;
	}

	static FName BattleScreenTag()
	{
		static const FName Name(TEXT("BattleScreen"));
		return Name;
	}
};
