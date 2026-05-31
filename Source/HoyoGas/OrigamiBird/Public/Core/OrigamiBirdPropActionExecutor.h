#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdMatchTypes.h"

class UOrigamiBirdMatchGameObject;

// Executes prop-specific board edits against a match.
// Prop effect classes decide which action to run; this executor owns the
// shared mutation details so UOrigamiBirdMatchGameObject does not grow one
// ApplyProp... method per prop.
struct HOYOGAS_API FOrigamiBirdPropActionExecutor
{
	static bool RemoveSingleTile(
		UOrigamiBirdMatchGameObject& Match,
		FIntPoint TargetPosition,
		bool bResolveAfterUse,
		FOrigamiBirdActionResult& OutResult);

	static bool RandomReplaceTile(
		UOrigamiBirdMatchGameObject& Match,
		FIntPoint TargetPosition,
		bool bResolveAfterUse,
		FOrigamiBirdActionResult& OutResult);

	static bool SwapColumns(
		UOrigamiBirdMatchGameObject& Match,
		int32 FirstColumn,
		int32 SecondColumn,
		bool bResolveAfterUse,
		FOrigamiBirdActionResult& OutResult);

	static bool CopyColumnToNeighbor(
		UOrigamiBirdMatchGameObject& Match,
		int32 SourceColumn,
		bool bResolveAfterUse,
		FOrigamiBirdActionResult& OutResult);

	static bool ShuffleBoard(
		UOrigamiBirdMatchGameObject& Match,
		bool bResolveAfterUse,
		FOrigamiBirdActionResult& OutResult);

	static bool ReplaceRandomTilesWithType(
		UOrigamiBirdMatchGameObject& Match,
		EOrigamiBirdTileType ReplacementTileType,
		int32 Count,
		bool bResolveAfterUse,
		FOrigamiBirdActionResult& OutResult);

	static bool Explode3x3(
		UOrigamiBirdMatchGameObject& Match,
		FIntPoint CenterPosition,
		bool bResolveAfterUse,
		FOrigamiBirdActionResult& OutResult);
};
