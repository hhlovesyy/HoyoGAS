#include "Core/OrigamiBirdBoardEditPropEffects.h"

#include "Core/OrigamiBirdMatchGameObject.h"

bool UOrigamiBirdRandomReplaceTilePropEffect::Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdPropUseResult& OutResult) const
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

	const bool bResolveAfterUse = GetBoolParam(Definition, TEXT("ResolveAfterUse"), Definition.bResolveAfterUse);
	return Match->ApplyPropRandomReplaceTile(Request.TargetPositions[0], bResolveAfterUse, OutResult);
}

bool UOrigamiBirdSwapColumnsPropEffect::Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdPropUseResult& OutResult) const
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

	const bool bResolveAfterUse = GetBoolParam(Definition, TEXT("ResolveAfterUse"), Definition.bResolveAfterUse);
	return Match->ApplyPropSwapColumns(Request.TargetColumns[0], Request.TargetColumns[1], bResolveAfterUse, OutResult);
}

bool UOrigamiBirdCopyColumnToNeighborPropEffect::Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdPropUseResult& OutResult) const
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

	const bool bResolveAfterUse = GetBoolParam(Definition, TEXT("ResolveAfterUse"), Definition.bResolveAfterUse);
	return Match->ApplyPropCopyColumnToNeighbor(Request.TargetColumns[0], bResolveAfterUse, OutResult);
}

bool UOrigamiBirdShuffleBoardPropEffect::Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdPropUseResult& OutResult) const
{
	(void)Request;

	if (!Match)
	{
		OutResult.FailureReasonId = TEXT("InvalidMatch");
		return false;
	}

	const bool bResolveAfterUse = GetBoolParam(Definition, TEXT("ResolveAfterUse"), Definition.bResolveAfterUse);
	return Match->ApplyPropShuffleBoard(bResolveAfterUse, OutResult);
}

bool UOrigamiBirdExplode3x3PropEffect::Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdPropUseResult& OutResult) const
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

	const bool bResolveAfterUse = GetBoolParam(Definition, TEXT("ResolveAfterUse"), Definition.bResolveAfterUse);
	return Match->ApplyPropExplode3x3(Request.TargetPositions[0], bResolveAfterUse, OutResult);
}
