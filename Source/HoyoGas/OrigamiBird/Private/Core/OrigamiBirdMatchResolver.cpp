#include "Core/OrigamiBirdMatchResolver.h"

DEFINE_LOG_CATEGORY_STATIC(LogOrigamiBirdMatchResolver, Log, All);

namespace
{
	TArray<FOrigamiBirdTile> MakeTileSnapshots(
		const FOrigamiBirdBoardState& BoardState,
		const TArray<FIntPoint>& Positions)
	{
		TArray<FOrigamiBirdTile> Result;
		Result.Reserve(Positions.Num());

		for (const FIntPoint& Position : Positions)
		{
			if (const FOrigamiBirdTile* Tile = BoardState.GetTile(Position))
			{
				Result.Add(*Tile);
				continue;
			}

			FOrigamiBirdTile EmptyTile;
			EmptyTile.BoardPosition = Position;
			Result.Add(EmptyTile);
		}

		return Result;
	}

	float GetComboScoreMultiplier(int32 ComboIndex)
	{
		if (ComboIndex <= 1)
		{
			return 1.0f;
		}

		if (ComboIndex == 2)
		{
			return 1.2f;
		}

		if (ComboIndex == 3)
		{
			return 1.5f;
		}

		return 2.0f;
	}
}

FOrigamiBirdMatchResolveResult FOrigamiBirdMatchResolver::ResolveCurrentMatches(
	FOrigamiBirdBoardState& BoardState,
	FOrigamiBirdCanMatchTileType CanMatchTileType,
	FOrigamiBirdCanFallTileType CanFallTileType,
	FOrigamiBirdGenerateTileType GenerateTileType,
	FOrigamiBirdGenerateTileId GenerateTileId,
	FOrigamiBirdGetTileScoreValue GetTileScoreValue,
	FOrigamiBirdMakeBoardSnapshot MakeSnapshot,
	FOrigamiBirdApplyResolveScore ApplyResolveScore,
	int32 MaxResolveIterations)
{
	FOrigamiBirdMatchResolveResult Result;
	int32 ResolveGuard = 0;

	while (true)
	{
		++ResolveGuard;
		if (ResolveGuard > MaxResolveIterations)
		{
			UE_LOG(LogOrigamiBirdMatchResolver, Warning, TEXT("ResolveCurrentMatches reached max iterations."));
			break;
		}

		const TArray<FIntPoint> Matches = FOrigamiBirdBoardResolver::FindAllMatches(BoardState, CanMatchTileType);
		if (Matches.IsEmpty())
		{
			break;
		}

		++Result.MaxComboIndex;

		FOrigamiBirdResolveCycle Cycle;
		Cycle.ComboIndex = Result.MaxComboIndex;
		Cycle.MatchPositions = Matches;
		Cycle.MatchedTiles = MakeTileSnapshots(BoardState, Matches);
		Cycle.RemovedTiles = Cycle.MatchedTiles;
		Cycle.RemovedTileCount = Matches.Num();
		Cycle.SnapshotBeforeRemove = MakeSnapshot();

		int32 BaseScore = 0;
		for (const FIntPoint& MatchPosition : Matches)
		{
			const FOrigamiBirdTile* Tile = BoardState.GetTile(MatchPosition);
			BaseScore += Tile ? GetTileScoreValue(Tile->TileType) : 0;
		}

		const int32 ScoreDelta = FMath::RoundToInt(
			static_cast<float>(BaseScore) * GetComboScoreMultiplier(Result.MaxComboIndex));
		Cycle.ScoreDelta = ScoreDelta;

		BoardState.RemoveTiles(Matches);
		ApplyResolveScore(ScoreDelta, Matches.Num(), Result.MaxComboIndex);

		Result.TotalScoreDelta += ScoreDelta;
		Result.RemovedTileCount += Matches.Num();
		Cycle.SnapshotAfterScore = MakeSnapshot();

		const FOrigamiBirdCollapseAndRefillResult CollapseResult =
			FOrigamiBirdBoardResolver::CollapseAndRefill(
				BoardState,
				CanFallTileType,
				GenerateTileType,
				GenerateTileId);

		Cycle.FallTransitions = CollapseResult.FallTransitions;
		Cycle.SpawnedTiles = CollapseResult.SpawnedTiles;
		Cycle.SpawnedPositions = CollapseResult.SpawnedPositions;
		Cycle.SnapshotAfterCollapse = MakeSnapshot();
		Result.ResolveCycles.Add(MoveTemp(Cycle));
	}

	return Result;
}
