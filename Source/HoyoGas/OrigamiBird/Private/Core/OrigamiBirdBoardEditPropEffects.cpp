#include "Core/OrigamiBirdBoardEditPropEffects.h"

#include "Core/OrigamiBirdMatchGameObject.h"

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

	return Match->ApplyPropRandomReplaceTile(Request.TargetPositions[0], Definition.bResolveAfterUse, OutResult);
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

	return Match->ApplyPropSwapColumns(Request.TargetColumns[0], Request.TargetColumns[1], Definition.bResolveAfterUse, OutResult);
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

	return Match->ApplyPropCopyColumnToNeighbor(Request.TargetColumns[0], Definition.bResolveAfterUse, OutResult);
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

	return Match->ApplyPropShuffleBoard(Definition.bResolveAfterUse, OutResult);
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

	return Match->ApplyPropExplode3x3(Request.TargetPositions[0], Definition.bResolveAfterUse, OutResult);
}
