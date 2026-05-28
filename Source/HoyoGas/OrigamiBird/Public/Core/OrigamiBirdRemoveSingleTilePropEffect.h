#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdPropEffect.h"
#include "OrigamiBirdRemoveSingleTilePropEffect.generated.h"

// 单格移除道具策略。
// 目标：选择一个棋盘格，将这个格子移除，然后触发下落补充；是否继续三消连锁由 ResolveAfterUse 参数控制。
UCLASS(BlueprintType, Blueprintable)
class HOYOGAS_API UOrigamiBirdRemoveSingleTilePropEffect : public UOrigamiBirdPropEffect
{
	GENERATED_BODY()

public:
	virtual bool Execute_Implementation(
		UOrigamiBirdMatchGameObject* Match,
		const FOrigamiBirdPropDefinitionRow& Definition,
		const FOrigamiBirdPropUseRequest& Request,
		FOrigamiBirdPropUseResult& OutResult) const override;
};
