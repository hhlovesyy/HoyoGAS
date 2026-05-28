#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdBoardResolver.h"
#include "Templates/Function.h"

using FOrigamiBirdGetTileScoreValue = TFunctionRef<int32(EOrigamiBirdTileType)>;
using FOrigamiBirdMakeBoardSnapshot = TFunctionRef<FOrigamiBirdBoardSnapshot()>;
using FOrigamiBirdApplyResolveScore = TFunctionRef<void(int32 ScoreDelta, int32 RemovedTileCount, int32 ComboIndex)>;

struct HOYOGAS_API FOrigamiBirdMatchResolveResult
{
	TArray<FOrigamiBirdResolveCycle> ResolveCycles;
	int32 TotalScoreDelta = 0;
	int32 RemovedTileCount = 0;
	int32 MaxComboIndex = 0;
};

// Resolves match cascades on an existing board.
// It owns match-cycle sequencing, while the game object owns session state updates.
struct HOYOGAS_API FOrigamiBirdMatchResolver
{
	static FOrigamiBirdMatchResolveResult ResolveCurrentMatches(
		FOrigamiBirdBoardState& BoardState,
		FOrigamiBirdCanMatchTileType CanMatchTileType,
		FOrigamiBirdCanFallTileType CanFallTileType,
		FOrigamiBirdGenerateTileType GenerateTileType,
		FOrigamiBirdGenerateTileId GenerateTileId,
		FOrigamiBirdGetTileScoreValue GetTileScoreValue,
		FOrigamiBirdMakeBoardSnapshot MakeSnapshot,
		FOrigamiBirdApplyResolveScore ApplyResolveScore,
		int32 MaxResolveIterations = 100);
};
