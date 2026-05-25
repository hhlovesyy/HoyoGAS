#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ElementTypes.h"
#include "ElementAuraConfigRow.generated.h"

USTRUCT(BlueprintType)
struct FElementAuraConfigRow : public FTableRowBase
{
	GENERATED_BODY()

	// 这行配置对应哪个元素
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGenshinElementType ElementType = EGenshinElementType::None;

	// 默认附着量。最小版里把它当作原神的 Gauge Unit（常见是 1 / 2 / 4）。
	// 运行时的 Duration 会按这个值统一推导，不再和下面的 Duration 字段分开驱动。
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultAuraValue = 1.0f;

	// 兼容旧表保留；运行时会自动同步为与 DefaultAuraValue 一致的参考值。
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultMaxValue = 1.0f;

	// 兼容旧表保留；运行时会按原神公式从 DefaultAuraValue 推导时长。
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultDuration = 10.0f;

	// 兼容旧表保留；运行时会自动同步为与 DefaultAuraValue 一致的参考时长。
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultMaxDuration = 10.0f;

	// 同元素再次命中时怎么刷新
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAuraRefreshPolicy RefreshPolicy = EAuraRefreshPolicy::RefreshDuration;

	// 无反应时，新元素是否覆盖旧元素
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanOverrideOtherAuraWhenNoReaction = true; //如果目标身上有别的 Aura，但这次没有触发反应，那能不能直接覆盖成新 Aura？
};
