#include "Core/OrigamiBirdTileEffect.h"

namespace
{
	bool IsSupportedClearMode(FName ClearMode)
	{
		return ClearMode == TEXT("Row")
			|| ClearMode == TEXT("Column")
			|| ClearMode == TEXT("Board")
			|| ClearMode == TEXT("Radius3x3")
			|| ClearMode == TEXT("Cross")
			|| ClearMode == TEXT("SameType");
	}

	bool CanRemoveTileAt(const FOrigamiBirdTileEffectContext& Context, FIntPoint Position)
	{
		const FOrigamiBirdTile* Tile = Context.BoardState.GetTile(Position);
		return Tile
			&& Tile->TileType != EOrigamiBirdTileType::None
			&& Context.CanRemoveTileType(Tile->TileType);
	}

	void AddBoardPositions(const FOrigamiBirdTileEffectContext& Context, FOrigamiBirdTileEffectResult& InOutResult)
	{
		for (int32 Y = 0; Y < Context.BoardState.GetHeight(); ++Y)
		{
			for (int32 X = 0; X < Context.BoardState.GetWidth(); ++X)
			{
				const FIntPoint Position(X, Y);
				if (CanRemoveTileAt(Context, Position))
				{
					InOutResult.AddRemovedPosition(Position);
				}
			}
		}
	}

	void AddRowPositions(const FOrigamiBirdTileEffectContext& Context, FOrigamiBirdTileEffectResult& InOutResult)
	{
		for (int32 X = 0; X < Context.BoardState.GetWidth(); ++X)
		{
			const FIntPoint Position(X, Context.TriggerPosition.Y);
			if (CanRemoveTileAt(Context, Position))
			{
				InOutResult.AddRemovedPosition(Position);
			}
		}
	}

	void AddColumnPositions(const FOrigamiBirdTileEffectContext& Context, FOrigamiBirdTileEffectResult& InOutResult)
	{
		for (int32 Y = 0; Y < Context.BoardState.GetHeight(); ++Y)
		{
			const FIntPoint Position(Context.TriggerPosition.X, Y);
			if (CanRemoveTileAt(Context, Position))
			{
				InOutResult.AddRemovedPosition(Position);
			}
		}
	}

	void AddRadius3x3Positions(const FOrigamiBirdTileEffectContext& Context, FOrigamiBirdTileEffectResult& InOutResult)
	{
		for (int32 Y = Context.TriggerPosition.Y - 1; Y <= Context.TriggerPosition.Y + 1; ++Y)
		{
			for (int32 X = Context.TriggerPosition.X - 1; X <= Context.TriggerPosition.X + 1; ++X)
			{
				const FIntPoint Position(X, Y);
				if (CanRemoveTileAt(Context, Position))
				{
					InOutResult.AddRemovedPosition(Position);
				}
			}
		}
	}

	void AddSameTypePositions(const FOrigamiBirdTileEffectContext& Context, FOrigamiBirdTileEffectResult& InOutResult)
	{
		const FOrigamiBirdTile* TriggerTile = Context.BoardState.GetTile(Context.TriggerPosition);
		if (!TriggerTile || TriggerTile->TileType == EOrigamiBirdTileType::None)
		{
			return;
		}

		for (int32 Y = 0; Y < Context.BoardState.GetHeight(); ++Y)
		{
			for (int32 X = 0; X < Context.BoardState.GetWidth(); ++X)
			{
				const FIntPoint Position(X, Y);
				const FOrigamiBirdTile* Tile = Context.BoardState.GetTile(Position);
				if (Tile && Tile->TileType == TriggerTile->TileType && CanRemoveTileAt(Context, Position))
				{
					InOutResult.AddRemovedPosition(Position);
				}
			}
		}
	}

	bool IsOtherTileSameType(const FOrigamiBirdTileEffectContext& Context)
	{
		const FOrigamiBirdTile* TriggerTile = Context.BoardState.GetTile(Context.TriggerPosition);
		const FOrigamiBirdTile* OtherTile = Context.BoardState.GetTile(Context.OtherPosition);
		return TriggerTile
			&& OtherTile
			&& TriggerTile->TileType != EOrigamiBirdTileType::None
			&& TriggerTile->TileType == OtherTile->TileType;
	}
}

bool UOrigamiBirdTileEffect::ValidateTrigger(const FOrigamiBirdTileTriggerDefinition& Trigger, FString& OutError) const
{
	(void)Trigger;
	(void)OutError;
	return true;
}

bool UOrigamiBirdTileEffect::Execute(
	const FOrigamiBirdTileEffectContext& Context,
	const FOrigamiBirdTileTriggerDefinition& Trigger,
	FOrigamiBirdTileEffectResult& InOutResult) const
{
	(void)Context;
	(void)Trigger;
	(void)InOutResult;
	return false;
}

bool UOrigamiBirdTileEffect::TryGetParamString(const FOrigamiBirdTileTriggerDefinition& Trigger, FName Key, FString& OutValue) const
{
	if (Key.IsNone())
	{
		return false;
	}

	for (const FOrigamiBirdTileEffectParam& Param : Trigger.EffectParams)
	{
		if (Param.Key == Key)
		{
			OutValue = Param.Value;
			return true;
		}
	}

	return false;
}

bool UOrigamiBirdTileEffect::TryGetBoolParam(const FOrigamiBirdTileTriggerDefinition& Trigger, FName Key, bool& OutValue) const
{
	FString RawValue;
	if (!TryGetParamString(Trigger, Key, RawValue))
	{
		return false;
	}

	RawValue = RawValue.TrimStartAndEnd();
	if (RawValue.Equals(TEXT("true"), ESearchCase::IgnoreCase)
		|| RawValue.Equals(TEXT("1"), ESearchCase::IgnoreCase)
		|| RawValue.Equals(TEXT("yes"), ESearchCase::IgnoreCase))
	{
		OutValue = true;
		return true;
	}

	if (RawValue.Equals(TEXT("false"), ESearchCase::IgnoreCase)
		|| RawValue.Equals(TEXT("0"), ESearchCase::IgnoreCase)
		|| RawValue.Equals(TEXT("no"), ESearchCase::IgnoreCase))
	{
		OutValue = false;
		return true;
	}

	return false;
}

bool UOrigamiBirdTileEffect::TryGetIntParam(const FOrigamiBirdTileTriggerDefinition& Trigger, FName Key, int32& OutValue) const
{
	FString RawValue;
	if (!TryGetParamString(Trigger, Key, RawValue))
	{
		return false;
	}

	return LexTryParseString(OutValue, *RawValue.TrimStartAndEnd());
}

bool UOrigamiBirdTileEffect::TryGetNameParam(const FOrigamiBirdTileTriggerDefinition& Trigger, FName Key, FName& OutValue) const
{
	FString RawValue;
	if (!TryGetParamString(Trigger, Key, RawValue))
	{
		return false;
	}

	RawValue = RawValue.TrimStartAndEnd();
	if (RawValue.IsEmpty())
	{
		return false;
	}

	OutValue = FName(*RawValue);
	return true;
}

bool UOrigamiBirdClearTilesByModeEffect::ValidateTrigger(const FOrigamiBirdTileTriggerDefinition& Trigger, FString& OutError) const
{
	FName ClearMode = NAME_None;
	if (!TryGetNameParam(Trigger, TEXT("ClearMode"), ClearMode))
	{
		OutError = TEXT("ClearMode is required");
		return false;
	}

	if (!IsSupportedClearMode(ClearMode))
	{
		OutError = FString::Printf(TEXT("Unsupported ClearMode=%s"), *ClearMode.ToString());
		return false;
	}

	FString RawRequireOtherSameTileType;
	if (TryGetParamString(Trigger, TEXT("RequireOtherSameTileType"), RawRequireOtherSameTileType))
	{
		bool bRequireOtherSameTileType = false;
		if (!TryGetBoolParam(Trigger, TEXT("RequireOtherSameTileType"), bRequireOtherSameTileType))
		{
			OutError = TEXT("RequireOtherSameTileType must be true/false");
			return false;
		}
	}

	return true;
}

bool UOrigamiBirdClearTilesByModeEffect::Execute(
	const FOrigamiBirdTileEffectContext& Context,
	const FOrigamiBirdTileTriggerDefinition& Trigger,
	FOrigamiBirdTileEffectResult& InOutResult) const
{
	if (!Context.BoardState.IsInsideBoard(Context.TriggerPosition))
	{
		return false;
	}

	bool bRequireOtherSameTileType = false;
	if (Context.TriggerType == EOrigamiBirdTileTriggerType::Swapped
		&& TryGetBoolParam(Trigger, TEXT("RequireOtherSameTileType"), bRequireOtherSameTileType)
		&& bRequireOtherSameTileType
		&& !IsOtherTileSameType(Context))
	{
		return false;
	}

	FName ClearMode = NAME_None;
	if (!TryGetNameParam(Trigger, TEXT("ClearMode"), ClearMode) || !IsSupportedClearMode(ClearMode))
	{
		return false;
	}

	if (ClearMode == TEXT("Row"))
	{
		AddRowPositions(Context, InOutResult);
		return true;
	}

	if (ClearMode == TEXT("Column"))
	{
		AddColumnPositions(Context, InOutResult);
		return true;
	}

	if (ClearMode == TEXT("Board"))
	{
		AddBoardPositions(Context, InOutResult);
		return true;
	}

	if (ClearMode == TEXT("Radius3x3"))
	{
		AddRadius3x3Positions(Context, InOutResult);
		return true;
	}

	if (ClearMode == TEXT("Cross"))
	{
		AddRowPositions(Context, InOutResult);
		AddColumnPositions(Context, InOutResult);
		return true;
	}

	if (ClearMode == TEXT("SameType"))
	{
		AddSameTypePositions(Context, InOutResult);
		return true;
	}

	return false;
}
