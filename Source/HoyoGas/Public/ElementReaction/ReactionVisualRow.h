#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ElementReaction/ElementTypes.h"
#include "ReactionVisualRow.generated.h"

USTRUCT(BlueprintType)
struct FReactionVisualRow : public FTableRowBase
{
	GENERATED_BODY()

	// 这行数据对应哪种反应
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EReactionType ReactionType = EReactionType::None;

	// 飘字显示文字，比如“蒸发”“融化”“冻结”
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ReactionDisplayText;

	// 这次反应的主颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ReactionColor = FLinearColor::White;

	// 飘字动画风格
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDamageTextStyleType DamageTextStyle = EDamageTextStyleType::Normal;

	// 是否启用冻结材质参数
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseFreezeMaterial = false;

	// 冻结材质参数值
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FreezeAmount = 0.f;

	// 后面用于查 FX 配置表的行名
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReactionFXRowName = NAME_None;
};