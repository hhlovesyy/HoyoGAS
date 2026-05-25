#include "GameplayDemoStatics.h"

#include "GameplayDemoSettings.h"
#include "Data/ItemDefinitionRow.h"
#include "Data/LevelProgressionRow.h"
#include "Engine/DataTable.h"

namespace
{
	TArray<FItemDefinitionRow> BuildDefaultItemDefinitions()
	{
		TArray<FItemDefinitionRow> Rows;

		FItemDefinitionRow RedGem;
		RedGem.ItemId = TEXT("RedGem");
		RedGem.DisplayName = INVTEXT("Red Gem");
		RedGem.TintColor = FLinearColor(1.0f, 0.2f, 0.2f);
		RedGem.ScoreValue = 25;
		RedGem.ShapeTypeTag = TEXT("Sphere");
		RedGem.Rarity = 1;
		Rows.Add(RedGem);

		FItemDefinitionRow BlueOrb;
		BlueOrb.ItemId = TEXT("BlueOrb");
		BlueOrb.DisplayName = INVTEXT("Blue Orb");
		BlueOrb.TintColor = FLinearColor(0.25f, 0.55f, 1.0f);
		BlueOrb.ScoreValue = 45;
		BlueOrb.ShapeTypeTag = TEXT("Cube");
		BlueOrb.Rarity = 2;
		Rows.Add(BlueOrb);

		FItemDefinitionRow GreenToken;
		GreenToken.ItemId = TEXT("GreenToken");
		GreenToken.DisplayName = INVTEXT("Green Token");
		GreenToken.TintColor = FLinearColor(0.25f, 1.0f, 0.45f);
		GreenToken.ScoreValue = 70;
		GreenToken.ShapeTypeTag = TEXT("Cone");
		GreenToken.Rarity = 3;
		Rows.Add(GreenToken);

		return Rows;
	}

	TArray<FLevelProgressionRow> BuildDefaultLevelRows()
	{
		TArray<FLevelProgressionRow> Rows;

		FLevelProgressionRow Level1;
		Level1.Level = 1;
		Level1.RequiredScore = 0;
		Level1.DisplayName = INVTEXT("Level 1");
		Level1.RewardText = INVTEXT("Base scavenger rank.");
		Rows.Add(Level1);

		FLevelProgressionRow Level2;
		Level2.Level = 2;
		Level2.RequiredScore = 100;
		Level2.DisplayName = INVTEXT("Level 2");
		Level2.RewardText = INVTEXT("Pickup streak bonus unlocked.");
		Rows.Add(Level2);

		FLevelProgressionRow Level3;
		Level3.Level = 3;
		Level3.RequiredScore = 250;
		Level3.DisplayName = INVTEXT("Level 3");
		Level3.RewardText = INVTEXT("High value loot appraisal unlocked.");
		Rows.Add(Level3);

		return Rows;
	}

	const UGameplayDemoSettings* GetGameplayDemoSettings()
	{
		return GetDefault<UGameplayDemoSettings>();
	}

	template <typename RowType>
	void SortRowsByStableKey(TArray<RowType>& Rows)
	{
		Rows.Sort([](const RowType& A, const RowType& B)
		{
			return A.RequiredScore < B.RequiredScore;
		});
	}
}

bool UGameplayDemoStatics::TryGetItemDefinition(const UObject* WorldContextObject, FName ItemId, FItemDefinitionRow& OutRow)
{
	if (ItemId.IsNone())
	{
		return false;
	}

	const UGameplayDemoSettings* Settings = GetGameplayDemoSettings();
	if (Settings && !Settings->ItemDefinitionTable.IsNull())
	{
		if (UDataTable* ItemTable = Settings->ItemDefinitionTable.LoadSynchronous())
		{
			if (const FItemDefinitionRow* FoundRow = ItemTable->FindRow<FItemDefinitionRow>(ItemId, TEXT("GameplayDemoStatics::TryGetItemDefinition")))
			{
				OutRow = *FoundRow;
				return true;
			}
		}
	}

	for (const FItemDefinitionRow& Row : BuildDefaultItemDefinitions())
	{
		if (Row.ItemId == ItemId)
		{
			OutRow = Row;
			return true;
		}
	}

	return false;
}

void UGameplayDemoStatics::GetAllItemDefinitions(const UObject* WorldContextObject, TArray<FItemDefinitionRow>& OutRows)
{
	OutRows.Reset();

	const UGameplayDemoSettings* Settings = GetGameplayDemoSettings();
	if (Settings && !Settings->ItemDefinitionTable.IsNull())
	{
		if (UDataTable* ItemTable = Settings->ItemDefinitionTable.LoadSynchronous())
		{
			static const FString Context = TEXT("GameplayDemoStatics::GetAllItemDefinitions");
			TArray<FItemDefinitionRow*> TableRows;
			ItemTable->GetAllRows(Context, TableRows);
			for (const FItemDefinitionRow* Row : TableRows)
			{
				if (Row)
				{
					OutRows.Add(*Row);
				}
			}
		}
	}

	if (OutRows.IsEmpty())
	{
		OutRows = BuildDefaultItemDefinitions();
	}
}

void UGameplayDemoStatics::GetAllLevelProgressionRows(const UObject* WorldContextObject, TArray<FLevelProgressionRow>& OutRows)
{
	OutRows.Reset();

	const UGameplayDemoSettings* Settings = GetGameplayDemoSettings();
	if (Settings && !Settings->LevelProgressionTable.IsNull())
	{
		if (UDataTable* LevelTable = Settings->LevelProgressionTable.LoadSynchronous())
		{
			static const FString Context = TEXT("GameplayDemoStatics::GetAllLevelProgressionRows");
			TArray<FLevelProgressionRow*> TableRows;
			LevelTable->GetAllRows(Context, TableRows);
			for (const FLevelProgressionRow* Row : TableRows)
			{
				if (Row)
				{
					OutRows.Add(*Row);
				}
			}
		}
	}

	if (OutRows.IsEmpty())
	{
		OutRows = BuildDefaultLevelRows();
	}

	OutRows.Sort([](const FLevelProgressionRow& A, const FLevelProgressionRow& B)
	{
		if (A.RequiredScore == B.RequiredScore)
		{
			return A.Level < B.Level;
		}
		return A.RequiredScore < B.RequiredScore;
	});
}

int32 UGameplayDemoStatics::GetItemScoreValue(const UObject* WorldContextObject, FName ItemId)
{
	FItemDefinitionRow Row;
	return TryGetItemDefinition(WorldContextObject, ItemId, Row) ? Row.ScoreValue : 0;
}
