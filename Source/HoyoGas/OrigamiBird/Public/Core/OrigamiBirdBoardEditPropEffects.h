#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdPropEffect.h"
#include "OrigamiBirdBoardEditPropEffects.generated.h"

// 将目标格随机替换成另一种普通方块。
UCLASS(BlueprintType, Blueprintable)
class HOYOGAS_API UOrigamiBirdRandomReplaceTilePropEffect : public UOrigamiBirdPropEffect
{
	GENERATED_BODY()

public:
	virtual bool ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const override;
	virtual bool Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdActionResult& OutResult) const override;
};

// 交换玩家选择的两列。
UCLASS(BlueprintType, Blueprintable)
class HOYOGAS_API UOrigamiBirdSwapColumnsPropEffect : public UOrigamiBirdPropEffect
{
	GENERATED_BODY()

public:
	virtual bool ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const override;
	virtual bool Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdActionResult& OutResult) const override;
};

// 复制目标列到相邻列：优先复制到右侧，最右列则复制到左侧。
UCLASS(BlueprintType, Blueprintable)
class HOYOGAS_API UOrigamiBirdCopyColumnToNeighborPropEffect : public UOrigamiBirdPropEffect
{
	GENERATED_BODY()

public:
	virtual bool ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const override;
	virtual bool Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdActionResult& OutResult) const override;
};

// 随机打乱棋盘，然后按配置决定是否立刻解算。
UCLASS(BlueprintType, Blueprintable)
class HOYOGAS_API UOrigamiBirdShuffleBoardPropEffect : public UOrigamiBirdPropEffect
{
	GENERATED_BODY()

public:
	virtual bool ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const override;
	virtual bool Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdActionResult& OutResult) const override;
};

// 炸掉点击点周围 3x3。直接爆炸本身不加分，是否触发后续连锁由参数控制。
UCLASS(BlueprintType, Blueprintable)
class HOYOGAS_API UOrigamiBirdReplaceRandomTilesPropEffect : public UOrigamiBirdPropEffect
{
	GENERATED_BODY()

public:
	virtual bool ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const override;
	virtual bool Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdActionResult& OutResult) const override;
};

UCLASS(BlueprintType, Blueprintable)
class HOYOGAS_API UOrigamiBirdExplode3x3PropEffect : public UOrigamiBirdPropEffect
{
	GENERATED_BODY()

public:
	virtual bool ValidateDefinition(const FOrigamiBirdPropDefinitionRow& Definition, FString& OutError) const override;
	virtual bool Execute_Implementation(UOrigamiBirdMatchGameObject* Match, const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdActionResult& OutResult) const override;
};
