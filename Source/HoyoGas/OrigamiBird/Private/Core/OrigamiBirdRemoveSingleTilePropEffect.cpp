#include "Core/OrigamiBirdRemoveSingleTilePropEffect.h"

#include "Core/OrigamiBirdMatchGameObject.h"
#include "Core/OrigamiBirdPropActionExecutor.h"

bool UOrigamiBirdRemoveSingleTilePropEffect::ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const
{
	if (Definition.TargetType != EOrigamiBirdPropTargetType::SingleTile)
	{
		OutError = FString::Printf(
			TEXT("TargetType=%d does not match expected TargetType=%d"),
			static_cast<int32>(Definition.TargetType),
			static_cast<int32>(EOrigamiBirdPropTargetType::SingleTile));
		return false;
	}

	return true;
}

bool UOrigamiBirdRemoveSingleTilePropEffect::Execute_Implementation(
	UOrigamiBirdMatchGameObject* Match,
	const FOrigamiBirdPropDefinitionRow& Definition,
	const FOrigamiBirdPropUseRequest& Request,
	FOrigamiBirdActionResult& OutResult) const
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

	return FOrigamiBirdPropActionExecutor::RemoveSingleTile(*Match, Request.TargetPositions[0], Definition.bResolveAfterUse, OutResult);
}
