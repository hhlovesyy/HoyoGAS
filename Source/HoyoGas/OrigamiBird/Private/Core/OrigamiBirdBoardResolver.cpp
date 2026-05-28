#include "Core/OrigamiBirdBoardResolver.h"

bool FOrigamiBirdBoardResolver::WouldCreateMatchAt(
	const FOrigamiBirdBoardState& BoardState,
	FIntPoint Position,
	FOrigamiBirdCanMatchTileType CanMatchTileType)
{
	checkf(BoardState.IsInsideBoard(Position), TEXT("WouldCreateMatchAt requires a valid board position."));

	const FOrigamiBirdTile* Tile = BoardState.GetTile(Position);
	if (!CanMatchTileType(Tile->TileType))
	{
		return false;
	}

	if (Position.X >= 2)
	{
		const FOrigamiBirdTile* Left1 = BoardState.GetTile(FIntPoint(Position.X - 1, Position.Y));
		const FOrigamiBirdTile* Left2 = BoardState.GetTile(FIntPoint(Position.X - 2, Position.Y));

		if (Left1->TileType == Tile->TileType
			&& Left2->TileType == Tile->TileType)
		{
			return true;
		}
	}

	if (Position.Y >= 2)
	{
		const FOrigamiBirdTile* Up1 = BoardState.GetTile(FIntPoint(Position.X, Position.Y - 1));
		const FOrigamiBirdTile* Up2 = BoardState.GetTile(FIntPoint(Position.X, Position.Y - 2));

		if (Up1->TileType == Tile->TileType
			&& Up2->TileType == Tile->TileType)
		{
			return true;
		}
	}

	return false;
}

TArray<FIntPoint> FOrigamiBirdBoardResolver::FindAllMatches(
	const FOrigamiBirdBoardState& BoardState,
	FOrigamiBirdCanMatchTileType CanMatchTileType)
{
	TSet<FIntPoint> MatchedSet;

	for (int32 Y = 0; Y < BoardState.GetHeight(); ++Y)
	{
		int32 RunStartX = 0;

		for (int32 X = 1; X <= BoardState.GetWidth(); ++X)
		{
			const EOrigamiBirdTileType RunType = BoardState.GetTile(FIntPoint(RunStartX, Y))->TileType;

			const bool bContinueRun =
				X < BoardState.GetWidth()
				&& BoardState.GetTile(FIntPoint(X, Y))->TileType == RunType
				&& CanMatchTileType(RunType);

			if (bContinueRun)
			{
				continue;
			}

			const int32 RunLength = X - RunStartX;

			if (CanMatchTileType(RunType) && RunLength >= 3)
			{
				for (int32 MatchX = RunStartX; MatchX < X; ++MatchX)
				{
					MatchedSet.Add(FIntPoint(MatchX, Y));
				}
			}

			RunStartX = X;
		}
	}

	for (int32 X = 0; X < BoardState.GetWidth(); ++X)
	{
		int32 RunStartY = 0;

		for (int32 Y = 1; Y <= BoardState.GetHeight(); ++Y)
		{
			const EOrigamiBirdTileType RunType = BoardState.GetTile(FIntPoint(X, RunStartY))->TileType;

			const bool bContinueRun =
				Y < BoardState.GetHeight()
				&& BoardState.GetTile(FIntPoint(X, Y))->TileType == RunType
				&& CanMatchTileType(RunType);

			if (bContinueRun)
			{
				continue;
			}

			const int32 RunLength = Y - RunStartY;

			if (CanMatchTileType(RunType) && RunLength >= 3)
			{
				for (int32 MatchY = RunStartY; MatchY < Y; ++MatchY)
				{
					MatchedSet.Add(FIntPoint(X, MatchY));
				}
			}

			RunStartY = Y;
		}
	}

	TArray<FIntPoint> MatchedPositions = MatchedSet.Array();

	MatchedPositions.Sort([](const FIntPoint& A, const FIntPoint& B)
	{
		return A.Y == B.Y ? A.X < B.X : A.Y < B.Y;
	});

	return MatchedPositions;
}

void FOrigamiBirdBoardResolver::GenerateInitialBoard(
	FOrigamiBirdBoardState& BoardState,
	int32 BoardWidth,
	int32 BoardHeight,
	FOrigamiBirdGenerateTileType GenerateTileType,
	FOrigamiBirdGenerateTileId GenerateTileId,
	FOrigamiBirdCanMatchTileType CanMatchTileType)
{
	BoardState.Initialize(BoardWidth, BoardHeight);

	for (int32 Y = 0; Y < BoardState.GetHeight(); ++Y)
	{
		for (int32 X = 0; X < BoardState.GetWidth(); ++X)
		{
			const FIntPoint Position(X, Y);
			FOrigamiBirdTile* Tile = BoardState.GetTile(Position);
			check(Tile);

			Tile->TileId = GenerateTileId();
			Tile->BoardPosition = Position;
			Tile->bIsSelected = false;

			int32 Guard = 0;
			do
			{
				Tile->TileType = GenerateTileType();
				++Guard;
			}
			while (WouldCreateMatchAt(BoardState, Position, CanMatchTileType) && Guard < 50);
		}
	}
}

FOrigamiBirdCollapseAndRefillResult FOrigamiBirdBoardResolver::CollapseAndRefill(
	FOrigamiBirdBoardState& BoardState,
	FOrigamiBirdCanFallTileType CanFallTileType,
	FOrigamiBirdGenerateTileType GenerateTileType,
	FOrigamiBirdGenerateTileId GenerateTileId)
{
	FOrigamiBirdCollapseAndRefillResult Result;

	for (int32 X = 0; X < BoardState.GetWidth(); ++X)
	{
		int32 SegmentBottomY = BoardState.GetHeight() - 1;

		while (SegmentBottomY >= 0)
		{
			const FOrigamiBirdTile* SegmentBottomTile = BoardState.GetTile(FIntPoint(X, SegmentBottomY));
			if (SegmentBottomTile
				&& SegmentBottomTile->TileType != EOrigamiBirdTileType::None
				&& !CanFallTileType(SegmentBottomTile->TileType))
			{
				--SegmentBottomY;
				continue;
			}

			int32 SegmentTopY = SegmentBottomY;
			while (SegmentTopY >= 0)
			{
				const FOrigamiBirdTile* Tile = BoardState.GetTile(FIntPoint(X, SegmentTopY));
				if (Tile && Tile->TileType != EOrigamiBirdTileType::None && !CanFallTileType(Tile->TileType))
				{
					break;
				}

				--SegmentTopY;
			}

			int32 WriteY = SegmentBottomY;
			for (int32 ReadY = SegmentBottomY; ReadY > SegmentTopY; --ReadY)
			{
				FOrigamiBirdTile* ReadTile = BoardState.GetTile(FIntPoint(X, ReadY));
				if (ReadTile && ReadTile->TileType != EOrigamiBirdTileType::None)
				{
					if (ReadY != WriteY)
					{
						const FIntPoint FromPosition(X, ReadY);
						const FIntPoint ToPosition(X, WriteY);

						FOrigamiBirdTileTransition Transition;
						Transition.TileId = ReadTile->TileId;
						Transition.TileType = ReadTile->TileType;
						Transition.FromPosition = FromPosition;
						Transition.ToPosition = ToPosition;
						Result.FallTransitions.Add(Transition);

						FOrigamiBirdTile* WriteTile = BoardState.GetTile(FIntPoint(X, WriteY));
						WriteTile->TileType = ReadTile->TileType;
						WriteTile->TileId = ReadTile->TileId;
						WriteTile->BoardPosition = ToPosition;
						WriteTile->bIsSelected = false;

						ReadTile->TileType = EOrigamiBirdTileType::None;
						ReadTile->TileId = INDEX_NONE;
						ReadTile->bIsSelected = false;
					}
					else
					{
						ReadTile->bIsSelected = false;
					}

					--WriteY;
				}
			}

			for (int32 Y = WriteY; Y > SegmentTopY; --Y)
			{
				FOrigamiBirdTile* Tile = BoardState.GetTile(FIntPoint(X, Y));
				Tile->TileType = GenerateTileType();
				Tile->TileId = GenerateTileId();
				Tile->BoardPosition = FIntPoint(X, Y);
				Tile->bIsSelected = false;

				Result.SpawnedTiles.Add(*Tile);
				Result.SpawnedPositions.Add(Tile->BoardPosition);
			}

			SegmentBottomY = SegmentTopY - 1;
		}
	}

	for (int32 Y = 0; Y < BoardState.GetHeight(); ++Y)
	{
		for (int32 X = 0; X < BoardState.GetWidth(); ++X)
		{
			if (FOrigamiBirdTile* Tile = BoardState.GetTile(FIntPoint(X, Y)))
			{
				Tile->BoardPosition = FIntPoint(X, Y);
			}
		}
	}

	return Result;
}
