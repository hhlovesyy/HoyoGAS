#include "Subsystems/OrigamiBirdMatchSubsystem.h"

#include "Core/OrigamiBirdMatchGameObject.h"
#include "Core/OrigamiBirdPropEffect.h"
#include "Engine/DataTable.h"
#include "OrigamiBirdSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogOrigamiBirdMatchSubsystem, Log, All);

namespace
{
	bool ValidatePropDefinition(FName PropId, const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError)
	{
		if (!Definition.EffectClass)
		{
			OutError = TEXT("EffectClass is not configured");
			return false;
		}

		if (Definition.InitialCount < 0)
		{
			OutError = TEXT("InitialCount must not be negative");
			return false;
		}

		if (Definition.MaxStackCount < 1)
		{
			OutError = TEXT("MaxStackCount must be at least 1");
			return false;
		}

		const UOrigamiBirdPropEffect* Effect = Definition.EffectClass->GetDefaultObject<UOrigamiBirdPropEffect>();
		if (!Effect)
		{
			OutError = TEXT("EffectClass is not a valid OrigamiBirdPropEffect");
			return false;
		}

		if (!Effect->ValidateDefinition(Definition, OutError))
		{
			OutError = FString::Printf(TEXT("%s. PropId=%s"), *OutError, *PropId.ToString());
			return false;
		}

		return true;
	}
}

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

	TArray<FName> PropIds;
	GetAllPropIds(PropIds);
	for (const FName& PropId : PropIds)
	{
		FOrigamiBirdPropDefinitionRow PropDefinition;
		if (!FindPropDefinition(PropId, PropDefinition))
		{
			continue;
		}

		if (PropDefinition.InitialCount <= 0)
		{
			continue;
		}

		ActiveMatch->GrantProp(
			PropId,
			PropDefinition.InitialCount,
			PropDefinition.bStackable,
			PropDefinition.MaxStackCount);
	}

	UE_LOG(
		LogOrigamiBirdMatchSubsystem,
		Log,
		TEXT("Started OrigamiBird match LevelId=%s Board=%dx%d Moves=%d TargetScore=%d TileDefinitions=%d PropDefinitions=%d"),
		*LevelId.ToString(),
		LevelDefinition.BoardWidth,
		LevelDefinition.BoardHeight,
		LevelDefinition.MoveLimit,
		LevelDefinition.TargetScore,
		TileDefinitions.Num(),
		PropIds.Num()
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

bool UOrigamiBirdMatchSubsystem::FindPropDefinition(FName PropId, FOrigamiBirdPropDefinitionRow& OutDefinition) const
{
	if (PropId.IsNone())
	{
		return false;
	}

	const UDataTable* PropTable = LoadPropDefinitionTable();
	if (!PropTable)
	{
		UE_LOG(LogOrigamiBirdMatchSubsystem, Warning, TEXT("FindPropDefinition failed because PropDefinitionTable is not configured."));
		return false;
	}

	const FString ContextString = TEXT("OrigamiBirdMatchSubsystem.FindPropDefinition");
	if (const FOrigamiBirdPropDefinitionRow* Row = PropTable->FindRow<FOrigamiBirdPropDefinitionRow>(PropId, ContextString))
	{
		FString ValidationError;
		if (!ValidatePropDefinition(PropId, *Row, ValidationError))
		{
			UE_LOG(
				LogOrigamiBirdMatchSubsystem,
				Error,
				TEXT("Prop definition '%s' is invalid: %s"),
				*PropId.ToString(),
				*ValidationError);
			return false;
		}

		if (!Row->bStackable && Row->MaxStackCount != 1)
		{
			UE_LOG(
				LogOrigamiBirdMatchSubsystem,
				Warning,
				TEXT("Prop definition '%s' is not stackable, MaxStackCount=%d will be treated as 1."),
				*PropId.ToString(),
				Row->MaxStackCount);
		}

		OutDefinition = *Row;
		return true;
	}

	return false;
}

void UOrigamiBirdMatchSubsystem::GetAllPropIds(TArray<FName>& OutPropIds) const
{
	OutPropIds.Reset();

	const UDataTable* PropTable = LoadPropDefinitionTable();
	if (!PropTable)
	{
		return;
	}

	OutPropIds = PropTable->GetRowNames();
}

void UOrigamiBirdMatchSubsystem::GetAllPropDefinitions(TArray<FOrigamiBirdPropDefinitionRow>& OutDefinitions) const
{
	OutDefinitions.Reset();

	const UDataTable* PropTable = LoadPropDefinitionTable();
	if (!PropTable)
	{
		return;
	}

	const FString ContextString = TEXT("OrigamiBirdMatchSubsystem.GetAllPropDefinitions");
	TArray<FOrigamiBirdPropDefinitionRow*> Rows;
	PropTable->GetAllRows<FOrigamiBirdPropDefinitionRow>(ContextString, Rows);

	OutDefinitions.Reserve(Rows.Num());
	for (const FOrigamiBirdPropDefinitionRow* Row : Rows)
	{
		if (Row)
		{
			OutDefinitions.Add(*Row);
		}
	}
}

bool UOrigamiBirdMatchSubsystem::GrantPropToActiveMatch(FName PropId, int32 Count)
{
	if (!ActiveMatch)
	{
		UE_LOG(LogOrigamiBirdMatchSubsystem, Warning, TEXT("GrantPropToActiveMatch failed because there is no active match."));
		return false;
	}

	FOrigamiBirdPropDefinitionRow PropDefinition;
	if (!FindPropDefinition(PropId, PropDefinition))
	{
		UE_LOG(LogOrigamiBirdMatchSubsystem, Warning, TEXT("GrantPropToActiveMatch failed because prop '%s' was not found."), *PropId.ToString());
		return false;
	}

	return ActiveMatch->GrantProp(PropId, Count, PropDefinition.bStackable, PropDefinition.MaxStackCount);
}

bool UOrigamiBirdMatchSubsystem::ConsumePropFromActiveMatch(FName PropId, int32 Count)
{
	if (!ActiveMatch)
	{
		UE_LOG(LogOrigamiBirdMatchSubsystem, Warning, TEXT("ConsumePropFromActiveMatch failed because there is no active match."));
		return false;
	}

	return ActiveMatch->ConsumeProp(PropId, Count);
}

bool UOrigamiBirdMatchSubsystem::UsePropOnActiveMatch(const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdActionResult& OutResult)
{
	OutResult = FOrigamiBirdActionResult();
	OutResult.ActionType = EOrigamiBirdActionType::UseProp;
	OutResult.PropId = Request.PropId;

	if (!ActiveMatch)
	{
		OutResult.FailureReasonId = TEXT("NoActiveMatch");
		UE_LOG(LogOrigamiBirdMatchSubsystem, Warning, TEXT("UsePropOnActiveMatch failed because there is no active match."));
		return false;
	}

	FOrigamiBirdPropDefinitionRow PropDefinition;
	if (!FindPropDefinition(Request.PropId, PropDefinition))
	{
		OutResult.FailureReasonId = TEXT("PropNotFound");
		UE_LOG(LogOrigamiBirdMatchSubsystem, Warning, TEXT("UsePropOnActiveMatch failed because prop '%s' was not found."), *Request.PropId.ToString());
		return false;
	}

	return ActiveMatch->UsePropWithResult(PropDefinition, Request, OutResult);
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

UDataTable* UOrigamiBirdMatchSubsystem::LoadPropDefinitionTable() const
{
	const UOrigamiBirdSettings* Settings = GetDefault<UOrigamiBirdSettings>();
	return Settings ? Settings->PropDefinitionTable.LoadSynchronous() : nullptr;
}
