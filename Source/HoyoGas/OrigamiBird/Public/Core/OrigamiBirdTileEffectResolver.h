#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdTileEffect.h"
#include "Templates/Function.h"

using FOrigamiBirdFindTileDefinition = TFunctionRef<const FOrigamiBirdTileDefinitionRow*(EOrigamiBirdTileType)>;

struct HOYOGAS_API FOrigamiBirdTileEffectResolver
{
	static TArray<FIntPoint> ExpandMatchedRemovePositions(
		const FOrigamiBirdBoardState& BoardState,
		const TArray<FIntPoint>& MatchPositions,
		FOrigamiBirdFindTileDefinition FindTileDefinition,
		FOrigamiBirdCanFallTileType CanRemoveTileType);

	static TArray<FIntPoint> ResolveSwapRemovePositions(
		const FOrigamiBirdBoardState& BoardState,
		FIntPoint From,
		FIntPoint To,
		FOrigamiBirdFindTileDefinition FindTileDefinition,
		FOrigamiBirdCanFallTileType CanRemoveTileType);
};
