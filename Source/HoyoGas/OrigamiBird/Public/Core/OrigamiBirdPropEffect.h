#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdMatchTypes.h"
#include "UObject/Object.h"
#include "OrigamiBirdPropEffect.generated.h"

class UOrigamiBirdMatchGameObject;

UCLASS(Abstract, BlueprintType, Blueprintable)
class HOYOGAS_API UOrigamiBirdPropEffect : public UObject
{
	GENERATED_BODY()

public:
	virtual bool ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "OrigamiBird|Props")
	bool Execute(
		UOrigamiBirdMatchGameObject* Match,
		const FOrigamiBirdPropDefinitionRow& Definition,
		const FOrigamiBirdPropUseRequest& Request,
		FOrigamiBirdActionResult& OutResult) const;
	virtual bool Execute_Implementation(
		UOrigamiBirdMatchGameObject* Match,
		const FOrigamiBirdPropDefinitionRow& Definition,
		const FOrigamiBirdPropUseRequest& Request,
		FOrigamiBirdActionResult& OutResult) const;

protected:
	bool TryGetParamString(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, FString& OutValue) const;
	bool TryGetBoolParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, bool& OutValue) const;
	bool TryGetIntParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, int32& OutValue) const;
	bool TryGetFloatParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, float& OutValue) const;
	bool TryGetNameParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, FName& OutValue) const;
	bool TryGetTileTypeParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, EOrigamiBirdTileType& OutValue) const;
};
