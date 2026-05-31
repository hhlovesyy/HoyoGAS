#include "Core/OrigamiBirdPropEffect.h"

bool UOrigamiBirdPropEffect::ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const
{
	(void)Definition;
	(void)OutError;
	return true;
}

bool UOrigamiBirdPropEffect::Execute_Implementation(
	UOrigamiBirdMatchGameObject* Match,
	const FOrigamiBirdPropDefinitionRow& Definition,
	const FOrigamiBirdPropUseRequest& Request,
	FOrigamiBirdActionResult& OutResult) const
{
	(void)Match;
	(void)Definition;

	OutResult = FOrigamiBirdActionResult();
	OutResult.ActionType = EOrigamiBirdActionType::UseProp;
	OutResult.PropId = Request.PropId;
	OutResult.FailureReasonId = TEXT("EffectNotImplemented");
	return false;
}

bool UOrigamiBirdPropEffect::TryGetParamString(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, FString& OutValue) const
{
	if (Key.IsNone())
	{
		return false;
	}

	for (const FOrigamiBirdPropEffectParam& Param : Definition.EffectParams)
	{
		if (Param.Key == Key)
		{
			OutValue = Param.Value;
			return true;
		}
	}

	return false;
}

bool UOrigamiBirdPropEffect::TryGetBoolParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, bool& OutValue) const
{
	FString RawValue;
	if (!TryGetParamString(Definition, Key, RawValue))
	{
		return false;
	}

	RawValue = RawValue.TrimStartAndEnd();
	if (RawValue.Equals(TEXT("true"), ESearchCase::IgnoreCase)
		|| RawValue.Equals(TEXT("1"), ESearchCase::IgnoreCase)
		|| RawValue.Equals(TEXT("yes"), ESearchCase::IgnoreCase))
	{
		OutValue = true;
		return true;
	}

	if (RawValue.Equals(TEXT("false"), ESearchCase::IgnoreCase)
		|| RawValue.Equals(TEXT("0"), ESearchCase::IgnoreCase)
		|| RawValue.Equals(TEXT("no"), ESearchCase::IgnoreCase))
	{
		OutValue = false;
		return true;
	}

	return false;
}

bool UOrigamiBirdPropEffect::TryGetIntParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, int32& OutValue) const
{
	FString RawValue;
	if (!TryGetParamString(Definition, Key, RawValue))
	{
		return false;
	}

	return LexTryParseString(OutValue, *RawValue.TrimStartAndEnd());
}

bool UOrigamiBirdPropEffect::TryGetFloatParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, float& OutValue) const
{
	FString RawValue;
	if (!TryGetParamString(Definition, Key, RawValue))
	{
		return false;
	}

	return LexTryParseString(OutValue, *RawValue.TrimStartAndEnd());
}

bool UOrigamiBirdPropEffect::TryGetNameParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, FName& OutValue) const
{
	FString RawValue;
	if (!TryGetParamString(Definition, Key, RawValue))
	{
		return false;
	}

	RawValue = RawValue.TrimStartAndEnd();
	if (RawValue.IsEmpty())
	{
		return false;
	}

	OutValue = FName(*RawValue);
	return true;
}

bool UOrigamiBirdPropEffect::TryGetTileTypeParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, EOrigamiBirdTileType& OutValue) const
{
	FString RawValue;
	if (!TryGetParamString(Definition, Key, RawValue))
	{
		return false;
	}

	RawValue = RawValue.TrimStartAndEnd();
	if (RawValue.IsEmpty())
	{
		return false;
	}

	const UEnum* TileTypeEnum = StaticEnum<EOrigamiBirdTileType>();
	if (!TileTypeEnum)
	{
		return false;
	}

	const int64 Value = TileTypeEnum->GetValueByNameString(RawValue);
	if (Value == INDEX_NONE)
	{
		return false;
	}

	OutValue = static_cast<EOrigamiBirdTileType>(Value);
	return true;
}
