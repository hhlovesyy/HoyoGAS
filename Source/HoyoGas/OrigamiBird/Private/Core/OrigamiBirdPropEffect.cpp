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

bool UOrigamiBirdPropEffect::GetBoolParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, bool DefaultValue) const
{
	FString RawValue;
	if (!TryGetParamString(Definition, Key, RawValue))
	{
		return DefaultValue;
	}

	RawValue = RawValue.TrimStartAndEnd();
	if (RawValue.Equals(TEXT("true"), ESearchCase::IgnoreCase)
		|| RawValue.Equals(TEXT("1"), ESearchCase::IgnoreCase)
		|| RawValue.Equals(TEXT("yes"), ESearchCase::IgnoreCase))
	{
		return true;
	}

	if (RawValue.Equals(TEXT("false"), ESearchCase::IgnoreCase)
		|| RawValue.Equals(TEXT("0"), ESearchCase::IgnoreCase)
		|| RawValue.Equals(TEXT("no"), ESearchCase::IgnoreCase))
	{
		return false;
	}

	return DefaultValue;
}

int32 UOrigamiBirdPropEffect::GetIntParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, int32 DefaultValue) const
{
	FString RawValue;
	if (!TryGetParamString(Definition, Key, RawValue))
	{
		return DefaultValue;
	}

	int32 ParsedValue = DefaultValue;
	return LexTryParseString(ParsedValue, *RawValue.TrimStartAndEnd()) ? ParsedValue : DefaultValue;
}

float UOrigamiBirdPropEffect::GetFloatParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, float DefaultValue) const
{
	FString RawValue;
	if (!TryGetParamString(Definition, Key, RawValue))
	{
		return DefaultValue;
	}

	float ParsedValue = DefaultValue;
	return LexTryParseString(ParsedValue, *RawValue.TrimStartAndEnd()) ? ParsedValue : DefaultValue;
}

FName UOrigamiBirdPropEffect::GetNameParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, FName DefaultValue) const
{
	FString RawValue;
	if (!TryGetParamString(Definition, Key, RawValue))
	{
		return DefaultValue;
	}

	RawValue = RawValue.TrimStartAndEnd();
	return RawValue.IsEmpty() ? DefaultValue : FName(*RawValue);
}

EOrigamiBirdTileType UOrigamiBirdPropEffect::GetTileTypeParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, EOrigamiBirdTileType DefaultValue) const
{
	FString RawValue;
	if (!TryGetParamString(Definition, Key, RawValue))
	{
		return DefaultValue;
	}

	RawValue = RawValue.TrimStartAndEnd();
	if (RawValue.IsEmpty())
	{
		return DefaultValue;
	}

	const UEnum* TileTypeEnum = StaticEnum<EOrigamiBirdTileType>();
	if (!TileTypeEnum)
	{
		return DefaultValue;
	}

	const int64 Value = TileTypeEnum->GetValueByNameString(RawValue);
	return Value == INDEX_NONE ? DefaultValue : static_cast<EOrigamiBirdTileType>(Value);
}
