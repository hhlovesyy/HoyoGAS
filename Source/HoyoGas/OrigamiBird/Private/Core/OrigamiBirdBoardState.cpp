#include "Core/OrigamiBirdBoardState.h"

void FOrigamiBirdBoardState::Reset()
{
	BoardWidth = 0;
	BoardHeight = 0;
	Tiles.Reset();
}

void FOrigamiBirdBoardState::Initialize(int32 InBoardWidth, int32 InBoardHeight)
{
	BoardWidth = InBoardWidth;
	BoardHeight = InBoardHeight;
	Tiles.SetNum(BoardWidth * BoardHeight);
}

int32 FOrigamiBirdBoardState::GetWidth() const
{
	return BoardWidth;
}

int32 FOrigamiBirdBoardState::GetHeight() const
{
	return BoardHeight;
}

const TArray<FOrigamiBirdTile>& FOrigamiBirdBoardState::GetTiles() const
{
	return Tiles;
}

TArray<FOrigamiBirdTile>& FOrigamiBirdBoardState::GetMutableTiles()
{
	return Tiles;
}

int32 FOrigamiBirdBoardState::ToIndex(FIntPoint Position) const
{
	return Position.Y * BoardWidth + Position.X;
}

bool FOrigamiBirdBoardState::IsInsideBoard(FIntPoint Position) const
{
	return Position.X >= 0
		&& Position.Y >= 0
		&& Position.X < BoardWidth
		&& Position.Y < BoardHeight;
}

bool FOrigamiBirdBoardState::IsInsideColumn(int32 Column) const
{
	return Column >= 0 && Column < BoardWidth;
}

const FOrigamiBirdTile* FOrigamiBirdBoardState::GetTile(FIntPoint Position) const
{
	const int32 Index = ToIndex(Position);
	return IsInsideBoard(Position) && Tiles.IsValidIndex(Index) ? &Tiles[Index] : nullptr;
}

FOrigamiBirdTile* FOrigamiBirdBoardState::GetTile(FIntPoint Position)
{
	const int32 Index = ToIndex(Position);
	return IsInsideBoard(Position) && Tiles.IsValidIndex(Index) ? &Tiles[Index] : nullptr;
}

void FOrigamiBirdBoardState::SwapTileData(FIntPoint A, FIntPoint B)
{
	FOrigamiBirdTile* TileA = GetTile(A);
	FOrigamiBirdTile* TileB = GetTile(B);

	if (!TileA || !TileB)
	{
		return;
	}

	Swap(TileA->TileType, TileB->TileType);
	Swap(TileA->TileId, TileB->TileId);
}

void FOrigamiBirdBoardState::RemoveTiles(const TArray<FIntPoint>& Positions)
{
	for (const FIntPoint& Position : Positions)
	{
		if (FOrigamiBirdTile* Tile = GetTile(Position))
		{
			Tile->TileType = EOrigamiBirdTileType::None;
			Tile->TileId = INDEX_NONE;
			Tile->bIsSelected = false;
		}
	}
}

void FOrigamiBirdBoardState::ClearSelection()
{
	for (FOrigamiBirdTile& Tile : Tiles)
	{
		Tile.bIsSelected = false;
	}
}
