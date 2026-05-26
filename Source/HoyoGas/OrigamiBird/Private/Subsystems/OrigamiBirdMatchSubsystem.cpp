#include "Subsystems/OrigamiBirdMatchSubsystem.h"

#include "Core/OrigamiBirdMatchGameObject.h"
#include "Engine/DataTable.h"
#include "OrigamiBirdSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogOrigamiBirdMatchSubsystem, Log, All);

void UOrigamiBirdMatchSubsystem::Deinitialize()
{
	EndActiveMatch();
	Super::Deinitialize();
}

UOrigamiBirdMatchGameObject* UOrigamiBirdMatchSubsystem::StartDefaultDevelopmentMatch()
{
	const UOrigamiBirdSettings* Settings = GetDefault<UOrigamiBirdSettings>();
	const FName LevelId = Settings ? Settings->DefaultDevelopmentLevelId : NAME_None;
	return StartMatchByLevelId(LevelId);
}

UOrigamiBirdMatchGameObject* UOrigamiBirdMatchSubsystem::StartMatchByLevelId(FName LevelId)
{
	if (LevelId.IsNone())
	{
		UE_LOG(LogOrigamiBirdMatchSubsystem, Warning, TEXT("StartMatchByLevelId failed because LevelId is None."));
		return nullptr;
	}

	FOrigamiBirdLevelDefinitionRow LevelDefinition;
	if (!FindLevelDefinition(LevelId, LevelDefinition))
	{
		UE_LOG(LogOrigamiBirdMatchSubsystem, Warning, TEXT("StartMatchByLevelId failed because level '%s' was not found."), *LevelId.ToString());
		return nullptr;
	}

	TArray<FOrigamiBirdTileDefinitionRow> TileDefinitions;
	GetAllTileDefinitions(TileDefinitions);
	if (TileDefinitions.IsEmpty())
	{
		UE_LOG(LogOrigamiBirdMatchSubsystem, Warning, TEXT("StartMatchByLevelId is starting without TileDefinition rows. Default tile rules will be used."));
	}

	EndActiveMatch();

	ActiveMatch = NewObject<UOrigamiBirdMatchGameObject>(this);
	ActiveMatch->InitializeFromLevelDefinition(LevelDefinition, TileDefinitions);

	UE_LOG(
		LogOrigamiBirdMatchSubsystem,
		Log,
		TEXT("Started OrigamiBird match LevelId=%s Board=%dx%d Moves=%d TargetScore=%d TileDefinitions=%d"),
		*LevelId.ToString(),
		LevelDefinition.BoardWidth,
		LevelDefinition.BoardHeight,
		LevelDefinition.MoveLimit,
		LevelDefinition.TargetScore,
		TileDefinitions.Num()
	);

	return ActiveMatch;
}

void UOrigamiBirdMatchSubsystem::EndActiveMatch()
{
	ActiveMatch = nullptr;
}

UOrigamiBirdMatchGameObject* UOrigamiBirdMatchSubsystem::GetActiveMatch() const
{
	return ActiveMatch;
}

bool UOrigamiBirdMatchSubsystem::FindLevelDefinition(FName LevelId, FOrigamiBirdLevelDefinitionRow& OutDefinition) const
{
	if (LevelId.IsNone())
	{
		return false;
	}

	const UDataTable* LevelTable = LoadLevelDefinitionTable();
	if (!LevelTable)
	{
		UE_LOG(LogOrigamiBirdMatchSubsystem, Warning, TEXT("FindLevelDefinition failed because LevelDefinitionTable is not configured."));
		return false;
	}

	const FString ContextString = TEXT("OrigamiBirdMatchSubsystem.FindLevelDefinition");
	if (const FOrigamiBirdLevelDefinitionRow* Row = LevelTable->FindRow<FOrigamiBirdLevelDefinitionRow>(LevelId, ContextString))
	{
		OutDefinition = *Row;
		return true;
	}

	return false;
}

bool UOrigamiBirdMatchSubsystem::FindTileDefinition(EOrigamiBirdTileType TileType, FOrigamiBirdTileDefinitionRow& OutDefinition) const
{
	TArray<FOrigamiBirdTileDefinitionRow> Definitions;
	GetAllTileDefinitions(Definitions);

	for (const FOrigamiBirdTileDefinitionRow& Definition : Definitions)
	{
		if (Definition.TileType == TileType)
		{
			OutDefinition = Definition;
			return true;
		}
	}

	return false;
}

void UOrigamiBirdMatchSubsystem::GetAllTileDefinitions(TArray<FOrigamiBirdTileDefinitionRow>& OutDefinitions) const
{
	OutDefinitions.Reset();

	const UDataTable* TileTable = LoadTileDefinitionTable();
	if (!TileTable)
	{
		return;
	}

	const FString ContextString = TEXT("OrigamiBirdMatchSubsystem.GetAllTileDefinitions");
	TArray<FOrigamiBirdTileDefinitionRow*> Rows;
	TileTable->GetAllRows<FOrigamiBirdTileDefinitionRow>(ContextString, Rows);

	OutDefinitions.Reserve(Rows.Num());
	for (const FOrigamiBirdTileDefinitionRow* Row : Rows)
	{
		if (Row)
		{
			OutDefinitions.Add(*Row);
		}
	}
}

void UOrigamiBirdMatchSubsystem::GetAllLevelIds(TArray<FName>& OutLevelIds) const
{
	OutLevelIds.Reset();

	const UDataTable* LevelTable = LoadLevelDefinitionTable();
	if (!LevelTable)
	{
		return;
	}

	OutLevelIds = LevelTable->GetRowNames();
}

UDataTable* UOrigamiBirdMatchSubsystem::LoadTileDefinitionTable() const
{
	const UOrigamiBirdSettings* Settings = GetDefault<UOrigamiBirdSettings>();
	return Settings ? Settings->TileDefinitionTable.LoadSynchronous() : nullptr;
}

UDataTable* UOrigamiBirdMatchSubsystem::LoadLevelDefinitionTable() const
{
	const UOrigamiBirdSettings* Settings = GetDefault<UOrigamiBirdSettings>();
	return Settings ? Settings->LevelDefinitionTable.LoadSynchronous() : nullptr;
}
