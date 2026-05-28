#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdBoardState.h"
#include "Templates/Function.h"

using FOrigamiBirdCanMatchTileType = TFunctionRef<bool(EOrigamiBirdTileType)>;
using FOrigamiBirdCanFallTileType = TFunctionRef<bool(EOrigamiBirdTileType)>;
using FOrigamiBirdGenerateTileType = TFunctionRef<EOrigamiBirdTileType()>;
using FOrigamiBirdGenerateTileId = TFunctionRef<int32()>;

struct HOYOGAS_API FOrigamiBirdCollapseAndRefillResult
{
	TArray<FOrigamiBirdTileTransition> FallTransitions;
	TArray<FOrigamiBirdTile> SpawnedTiles;
	TArray<FIntPoint> SpawnedPositions;
};

// Pure board-rule algorithms. It reads and writes board state, but does not own
// match session data such as score, moves, phase, props, or presentation steps.
struct HOYOGAS_API FOrigamiBirdBoardResolver
{
	static bool WouldCreateMatchAt(
		const FOrigamiBirdBoardState& BoardState,
		FIntPoint Position,
		FOrigamiBirdCanMatchTileType CanMatchTileType);

	static TArray<FIntPoint> FindAllMatches(
		const FOrigamiBirdBoardState& BoardState,
		FOrigamiBirdCanMatchTileType CanMatchTileType);

	static void GenerateInitialBoard(
		FOrigamiBirdBoardState& BoardState,
		int32 BoardWidth,
		int32 BoardHeight,
		FOrigamiBirdGenerateTileType GenerateTileType,
		FOrigamiBirdGenerateTileId GenerateTileId,
		FOrigamiBirdCanMatchTileType CanMatchTileType);

	static FOrigamiBirdCollapseAndRefillResult CollapseAndRefill(
		FOrigamiBirdBoardState& BoardState,
		FOrigamiBirdCanFallTileType CanFallTileType,
		FOrigamiBirdGenerateTileType GenerateTileType,
		FOrigamiBirdGenerateTileId GenerateTileId);
};
