#include "Core/OrigamiBirdPropActionExecutor.h"

#include "Core/OrigamiBirdMatchGameObject.h"

bool FOrigamiBirdPropActionExecutor::RemoveSingleTile(
	UOrigamiBirdMatchGameObject& Match,
	FIntPoint TargetPosition,
	bool bResolveAfterUse,
	FOrigamiBirdActionResult& OutResult)
{
	if (!Match.BoardState.IsInsideBoard(TargetPosition))
	{
		OutResult.FailureReasonId = TEXT("OutOfBoard");
		return false;
	}

	const FOrigamiBirdTile* TargetTile = Match.BoardState.GetTile(TargetPosition);
	if (!TargetTile || TargetTile->TileType == EOrigamiBirdTileType::None)
	{
		OutResult.FailureReasonId = TEXT("EmptyTile");
		return false;
	}

	TArray<FIntPoint> RemovedPositions;
	RemovedPositions.Add(TargetPosition);
	OutResult.BoardChangeSteps.Add(Match.MakeRemoveStep(RemovedPositions));

	Match.BoardState.RemoveTiles(RemovedPositions);
	Match.RemovedTileCount += RemovedPositions.Num();
	OutResult.RemovedTileCount += RemovedPositions.Num();
	Match.AppendCollapseSteps(Match.CollapseAndRefill(), OutResult);

	if (bResolveAfterUse)
	{
		Match.ResolveAfterPropUse(OutResult);
	}

	OutResult.bAccepted = true;
	return true;
}

bool FOrigamiBirdPropActionExecutor::RandomReplaceTile(
	UOrigamiBirdMatchGameObject& Match,
	FIntPoint TargetPosition,
	bool bResolveAfterUse,
	FOrigamiBirdActionResult& OutResult)
{
	if (!Match.BoardState.IsInsideBoard(TargetPosition))
	{
		OutResult.FailureReasonId = TEXT("OutOfBoard");
		return false;
	}

	FOrigamiBirdTile* TargetTile = Match.BoardState.GetTile(TargetPosition);
	if (!TargetTile || TargetTile->TileType == EOrigamiBirdTileType::None)
	{
		OutResult.FailureReasonId = TEXT("EmptyTile");
		return false;
	}

	TArray<FIntPoint> Positions;
	Positions.Add(TargetPosition);
	OutResult.BoardChangeSteps.Add(Match.MakeRemoveStep(Positions));

	TargetTile->TileType = Match.GenerateRandomTileTypeExcept(TargetTile->TileType);
	TargetTile->TileId = Match.NextTileId++;
	TargetTile->BoardPosition = TargetPosition;
	TargetTile->bIsSelected = false;
	OutResult.BoardChangeSteps.Add(Match.MakeSpawnStep({ *TargetTile }, Positions));

	if (bResolveAfterUse)
	{
		Match.ResolveAfterPropUse(OutResult);
	}

	OutResult.bAccepted = true;
	return true;
}

bool FOrigamiBirdPropActionExecutor::SwapColumns(
	UOrigamiBirdMatchGameObject& Match,
	int32 FirstColumn,
	int32 SecondColumn,
	bool bResolveAfterUse,
	FOrigamiBirdActionResult& OutResult)
{
	if (!Match.BoardState.IsInsideColumn(FirstColumn) || !Match.BoardState.IsInsideColumn(SecondColumn))
	{
		OutResult.FailureReasonId = TEXT("OutOfBoard");
		return false;
	}

	if (FirstColumn == SecondColumn)
	{
		OutResult.FailureReasonId = TEXT("SameColumn");
		return false;
	}

	FOrigamiBirdBoardChangeStep SwapStep;
	SwapStep.StepType = EOrigamiBirdBoardChangeStepType::Swap;

	for (int32 Y = 0; Y < Match.StartParams.BoardHeight; ++Y)
	{
		const FIntPoint FirstPosition(FirstColumn, Y);
		const FIntPoint SecondPosition(SecondColumn, Y);
		const FOrigamiBirdTile* FirstTile = Match.BoardState.GetTile(FirstPosition);
		const FOrigamiBirdTile* SecondTile = Match.BoardState.GetTile(SecondPosition);

		if (FirstTile && FirstTile->TileId != INDEX_NONE)
		{
			FOrigamiBirdTileTransition Transition;
			Transition.TileId = FirstTile->TileId;
			Transition.TileType = FirstTile->TileType;
			Transition.FromPosition = FirstPosition;
			Transition.ToPosition = SecondPosition;
			SwapStep.TileTransitions.Add(Transition);
		}

		if (SecondTile && SecondTile->TileId != INDEX_NONE)
		{
			FOrigamiBirdTileTransition Transition;
			Transition.TileId = SecondTile->TileId;
			Transition.TileType = SecondTile->TileType;
			Transition.FromPosition = SecondPosition;
			Transition.ToPosition = FirstPosition;
			SwapStep.TileTransitions.Add(Transition);
		}
	}

	OutResult.BoardChangeSteps.Add(MoveTemp(SwapStep));

	for (int32 Y = 0; Y < Match.StartParams.BoardHeight; ++Y)
	{
		Match.BoardState.SwapTileData(FIntPoint(FirstColumn, Y), FIntPoint(SecondColumn, Y));
	}

	if (bResolveAfterUse)
	{
		Match.ResolveAfterPropUse(OutResult);
	}

	OutResult.bAccepted = true;
	return true;
}

bool FOrigamiBirdPropActionExecutor::CopyColumnToNeighbor(
	UOrigamiBirdMatchGameObject& Match,
	int32 SourceColumn,
	bool bResolveAfterUse,
	FOrigamiBirdActionResult& OutResult)
{
	if (!Match.BoardState.IsInsideColumn(SourceColumn))
	{
		OutResult.FailureReasonId = TEXT("OutOfBoard");
		return false;
	}

	const int32 TargetColumn = Match.BoardState.IsInsideColumn(SourceColumn + 1) ? SourceColumn + 1 : SourceColumn - 1;
	if (!Match.BoardState.IsInsideColumn(TargetColumn))
	{
		OutResult.FailureReasonId = TEXT("NoNeighborColumn");
		return false;
	}

	TArray<FIntPoint> TargetPositions;
	for (int32 Y = 0; Y < Match.StartParams.BoardHeight; ++Y)
	{
		TargetPositions.Add(FIntPoint(TargetColumn, Y));
	}
	OutResult.BoardChangeSteps.Add(Match.MakeRemoveStep(TargetPositions));

	TArray<FOrigamiBirdTile> SpawnedTiles;
	TArray<FIntPoint> SpawnedPositions;

	for (int32 Y = 0; Y < Match.StartParams.BoardHeight; ++Y)
	{
		const FOrigamiBirdTile* SourceTile = Match.BoardState.GetTile(FIntPoint(SourceColumn, Y));
		FOrigamiBirdTile* TargetTile = Match.BoardState.GetTile(FIntPoint(TargetColumn, Y));
		if (!SourceTile || !TargetTile)
		{
			continue;
		}

		TargetTile->TileType = SourceTile->TileType;
		TargetTile->TileId = Match.NextTileId++;
		TargetTile->BoardPosition = FIntPoint(TargetColumn, Y);
		TargetTile->bIsSelected = false;

		SpawnedTiles.Add(*TargetTile);
		SpawnedPositions.Add(TargetTile->BoardPosition);
	}

	OutResult.BoardChangeSteps.Add(Match.MakeSpawnStep(SpawnedTiles, SpawnedPositions));

	if (bResolveAfterUse)
	{
		Match.ResolveAfterPropUse(OutResult);
	}

	OutResult.bAccepted = true;
	return true;
}

bool FOrigamiBirdPropActionExecutor::ShuffleBoard(
	UOrigamiBirdMatchGameObject& Match,
	bool bResolveAfterUse,
	FOrigamiBirdActionResult& OutResult)
{
	TArray<FIntPoint> Positions;
	TArray<EOrigamiBirdTileType> TileTypes;

	for (int32 Y = 0; Y < Match.StartParams.BoardHeight; ++Y)
	{
		for (int32 X = 0; X < Match.StartParams.BoardWidth; ++X)
		{
			const FIntPoint Position(X, Y);
			const FOrigamiBirdTile* Tile = Match.BoardState.GetTile(Position);
			if (Tile && Tile->TileType != EOrigamiBirdTileType::None && Match.CanFallTileType(Tile->TileType))
			{
				Positions.Add(Position);
				TileTypes.Add(Tile->TileType);
			}
		}
	}

	if (Positions.Num() <= 1)
	{
		OutResult.FailureReasonId = TEXT("NotEnoughTiles");
		return false;
	}

	OutResult.BoardChangeSteps.Add(Match.MakeRemoveStep(Positions));

	for (int32 Index = TileTypes.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = Match.RandomStream.RandRange(0, Index);
		TileTypes.Swap(Index, SwapIndex);
	}

	TArray<FOrigamiBirdTile> SpawnedTiles;
	TArray<FIntPoint> SpawnedPositions;

	for (int32 Index = 0; Index < Positions.Num(); ++Index)
	{
		FOrigamiBirdTile* Tile = Match.BoardState.GetTile(Positions[Index]);
		if (!Tile)
		{
			continue;
		}

		Tile->TileType = TileTypes[Index];
		Tile->TileId = Match.NextTileId++;
		Tile->BoardPosition = Positions[Index];
		Tile->bIsSelected = false;

		SpawnedTiles.Add(*Tile);
		SpawnedPositions.Add(Tile->BoardPosition);
	}

	OutResult.BoardChangeSteps.Add(Match.MakeSpawnStep(SpawnedTiles, SpawnedPositions));

	if (bResolveAfterUse)
	{
		Match.ResolveAfterPropUse(OutResult);
	}

	OutResult.bAccepted = true;
	return true;
}

bool FOrigamiBirdPropActionExecutor::Explode3x3(
	UOrigamiBirdMatchGameObject& Match,
	FIntPoint CenterPosition,
	bool bResolveAfterUse,
	FOrigamiBirdActionResult& OutResult)
{
	if (!Match.BoardState.IsInsideBoard(CenterPosition))
	{
		OutResult.FailureReasonId = TEXT("OutOfBoard");
		return false;
	}

	TArray<FIntPoint> RemovedPositions;
	for (int32 Y = CenterPosition.Y - 1; Y <= CenterPosition.Y + 1; ++Y)
	{
		for (int32 X = CenterPosition.X - 1; X <= CenterPosition.X + 1; ++X)
		{
			const FIntPoint Position(X, Y);
			const FOrigamiBirdTile* Tile = Match.BoardState.GetTile(Position);
			if (Tile && Tile->TileType != EOrigamiBirdTileType::None && Match.CanFallTileType(Tile->TileType))
			{
				RemovedPositions.Add(Position);
			}
		}
	}

	if (RemovedPositions.IsEmpty())
	{
		OutResult.FailureReasonId = TEXT("NoTilesToRemove");
		return false;
	}

	OutResult.BoardChangeSteps.Add(Match.MakeRemoveStep(RemovedPositions));

	Match.BoardState.RemoveTiles(RemovedPositions);
	Match.RemovedTileCount += RemovedPositions.Num();
	OutResult.RemovedTileCount += RemovedPositions.Num();
	Match.AppendCollapseSteps(Match.CollapseAndRefill(), OutResult);

	if (bResolveAfterUse)
	{
		Match.ResolveAfterPropUse(OutResult);
	}

	OutResult.bAccepted = true;
	return true;
}
