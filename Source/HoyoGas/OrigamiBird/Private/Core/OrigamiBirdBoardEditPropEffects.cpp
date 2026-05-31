#include "Core/OrigamiBirdBoardEditPropEffects.h"

#include "Core/OrigamiBirdMatchGameObject.h"
#include "Core/OrigamiBirdPropActionExecutor.h"

namespace
{
	bool ValidateExpectedTargetType(
		const FOrigamiBirdPropDefinitionRow& Definition,
		EOrigamiBirdPropTargetType ExpectedTargetType,
		FString& OutError)
	{
		if (Definition.TargetType != ExpectedTargetType)
		{
			OutError = FString::Printf(
				TEXT("TargetType=%d does not match expected TargetType=%d"),
				static_cast<int32>(Definition.TargetType),
				static_cast<int32>(ExpectedTargetType));
			return false;
		}

		return true;
	}
}

bool UOrigamiBirdRandomReplaceTilePropEffect::ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const
{
	return ValidateExpectedTargetType(Definition, EOrigamiBirdPropTargetType::SingleTile, OutError);
}

bool UOrigamiBirdRandomReplaceTilePropEffect::Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdActionResult& OutResult) const
{
	if (!Match)
	{
		OutResult.FailureReasonId = TEXT("InvalidMatch");
		return false;
	}
	if (Request.TargetPositions.Num() < 1)
	{
		OutResult.FailureReasonId = TEXT("MissingTargetTile");
		return false;
	}

	return FOrigamiBirdPropActionExecutor::RandomReplaceTile(*Match, Request.TargetPositions[0], Definition.bResolveAfterUse, OutResult);
}

bool UOrigamiBirdSwapColumnsPropEffect::ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const
{
	return ValidateExpectedTargetType(Definition, EOrigamiBirdPropTargetType::TwoColumns, OutError);
}

bool UOrigamiBirdSwapColumnsPropEffect::Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdActionResult& OutResult) const
{
	if (!Match)
	{
		OutResult.FailureReasonId = TEXT("InvalidMatch");
		return false;
	}
	if (Request.TargetColumns.Num() < 2)
	{
		OutResult.FailureReasonId = TEXT("MissingTargetColumns");
		return false;
	}

	return FOrigamiBirdPropActionExecutor::SwapColumns(*Match, Request.TargetColumns[0], Request.TargetColumns[1], Definition.bResolveAfterUse, OutResult);
}

bool UOrigamiBirdCopyColumnToNeighborPropEffect::ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const
{
	return ValidateExpectedTargetType(Definition, EOrigamiBirdPropTargetType::SingleColumn, OutError);
}

bool UOrigamiBirdCopyColumnToNeighborPropEffect::Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdActionResult& OutResult) const
{
	if (!Match)
	{
		OutResult.FailureReasonId = TEXT("InvalidMatch");
		return false;
	}
	if (Request.TargetColumns.Num() < 1)
	{
		OutResult.FailureReasonId = TEXT("MissingTargetColumn");
		return false;
	}

	return FOrigamiBirdPropActionExecutor::CopyColumnToNeighbor(*Match, Request.TargetColumns[0], Definition.bResolveAfterUse, OutResult);
}

bool UOrigamiBirdShuffleBoardPropEffect::ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const
{
	return ValidateExpectedTargetType(Definition, EOrigamiBirdPropTargetType::None, OutError);
}

bool UOrigamiBirdShuffleBoardPropEffect::Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdActionResult& OutResult) const
{
	(void)Request;

	if (!Match)
	{
		OutResult.FailureReasonId = TEXT("InvalidMatch");
		return false;
	}

	return FOrigamiBirdPropActionExecutor::ShuffleBoard(*Match, Definition.bResolveAfterUse, OutResult);
}

bool UOrigamiBirdReplaceRandomTilesPropEffect::ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const
{
	if (!ValidateExpectedTargetType(Definition, EOrigamiBirdPropTargetType::None, OutError))
	{
		return false;
	}

	EOrigamiBirdTileType ReplacementTileType = EOrigamiBirdTileType::None;
	if (!TryGetTileTypeParam(Definition, TEXT("TileType"), ReplacementTileType)
		|| ReplacementTileType == EOrigamiBirdTileType::None)
	{
		OutError = TEXT("EffectParams.TileType must be configured");
		return false;
	}

	int32 Count = 0;
	if (!TryGetIntParam(Definition, TEXT("Count"), Count) || Count <= 0)
	{
		OutError = TEXT("EffectParams.Count must be configured and greater than 0");
		return false;
	}

	return true;
}

bool UOrigamiBirdReplaceRandomTilesPropEffect::Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdActionResult& OutResult) const
{
	(void)Request;

	if (!Match)
	{
		OutResult.FailureReasonId = TEXT("InvalidMatch");
		return false;
	}

	EOrigamiBirdTileType ReplacementTileType = EOrigamiBirdTileType::None;
	if (!TryGetTileTypeParam(Definition, TEXT("TileType"), ReplacementTileType)
		|| ReplacementTileType == EOrigamiBirdTileType::None)
	{
		OutResult.FailureReasonId = TEXT("MissingReplacementTileType");
		return false;
	}

	int32 Count = 0;
	if (!TryGetIntParam(Definition, TEXT("Count"), Count) || Count <= 0)
	{
		OutResult.FailureReasonId = TEXT("InvalidReplacementCount");
		return false;
	}

	return FOrigamiBirdPropActionExecutor::ReplaceRandomTilesWithType(
		*Match,
		ReplacementTileType,
		Count,
		Definition.bResolveAfterUse,
		OutResult);
}

bool UOrigamiBirdExplode3x3PropEffect::ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const
{
	return ValidateExpectedTargetType(Definition, EOrigamiBirdPropTargetType::SingleTile, OutError);
}

bool UOrigamiBirdExplode3x3PropEffect::Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdActionResult& OutResult) const
{
	if (!Match)
	{
		OutResult.FailureReasonId = TEXT("InvalidMatch");
		return false;
	}
	if (Request.TargetPositions.Num() < 1)
	{
		OutResult.FailureReasonId = TEXT("MissingTargetTile");
		return false;
	}

	return FOrigamiBirdPropActionExecutor::Explode3x3(*Match, Request.TargetPositions[0], Definition.bResolveAfterUse, OutResult);
}
