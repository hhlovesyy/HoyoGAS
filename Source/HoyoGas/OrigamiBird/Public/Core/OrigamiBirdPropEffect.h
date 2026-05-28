#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdMatchTypes.h"
#include "UObject/Object.h"
#include "OrigamiBirdPropEffect.generated.h"

class UOrigamiBirdMatchGameObject;

// 道具效果策略基类。
// 每个具体道具效果继承它，并在 Execute 里修改棋盘、填充 Result。
UCLASS(Abstract, BlueprintType, Blueprintable)
class HOYOGAS_API UOrigamiBirdPropEffect : public UObject
{
	GENERATED_BODY()

public:
	virtual bool ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const;

	// 执行道具效果。通常使用类默认对象 CDO 调用，不要在每次使用道具时反复 new 策略对象。
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
	// 从 EffectParams 里读取原始字符串。找不到时返回 false。
	bool TryGetParamString(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, FString& OutValue) const;

	// 常用参数读取辅助函数。策略子类只关心业务，不需要每次重复解析字符串。
	bool GetBoolParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, bool DefaultValue = false) const;
	int32 GetIntParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, int32 DefaultValue = 0) const;
	float GetFloatParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, float DefaultValue = 0.0f) const;
	FName GetNameParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, FName DefaultValue = NAME_None) const;
	EOrigamiBirdTileType GetTileTypeParam(const FOrigamiBirdPropDefinitionRow& Definition, FName Key, EOrigamiBirdTileType DefaultValue = EOrigamiBirdTileType::None) const;
};
