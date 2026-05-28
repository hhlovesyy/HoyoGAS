#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdMatchTypes.h"

// Owns the board grid data and basic coordinate-based mutations.
// Match rules, score, moves, props, and UI presentation stay outside this type.
struct HOYOGAS_API FOrigamiBirdBoardState
{
public:
	void Reset();
	void Initialize(int32 InBoardWidth, int32 InBoardHeight);

	int32 GetWidth() const;
	int32 GetHeight() const;
	const TArray<FOrigamiBirdTile>& GetTiles() const;
	TArray<FOrigamiBirdTile>& GetMutableTiles();

	int32 ToIndex(FIntPoint Position) const;
	bool IsInsideBoard(FIntPoint Position) const;
	bool IsInsideColumn(int32 Column) const;

	const FOrigamiBirdTile* GetTile(FIntPoint Position) const;
	FOrigamiBirdTile* GetTile(FIntPoint Position);

	void SwapTileData(FIntPoint A, FIntPoint B);
	void RemoveTiles(const TArray<FIntPoint>& Positions);
	void ClearSelection();

private:
	int32 BoardWidth = 0;
	int32 BoardHeight = 0;
	TArray<FOrigamiBirdTile> Tiles;
};
