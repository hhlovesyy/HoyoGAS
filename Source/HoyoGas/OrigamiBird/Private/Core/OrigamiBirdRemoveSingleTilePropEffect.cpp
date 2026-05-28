#include "Core/OrigamiBirdRemoveSingleTilePropEffect.h"

#include "Core/OrigamiBirdMatchGameObject.h"

bool UOrigamiBirdRemoveSingleTilePropEffect::Execute_Implementation(
	UOrigamiBirdMatchGameObject* Match,
	const FOrigamiBirdPropDefinitionRow& Definition,
	const FOrigamiBirdPropUseRequest& Request,
	FOrigamiBirdPropUseResult& OutResult) const
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
	return Match->ApplyPropRemoveSingleTile(Request.TargetPositions[0], bResolveAfterUse, OutResult);
}
