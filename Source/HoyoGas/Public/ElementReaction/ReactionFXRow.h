#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ReactionFXRow.generated.h"

class UParticleSystem;

USTRUCT(BlueprintType)
struct FReactionFXRow : public FTableRowBase
{
	GENERATED_BODY()

	// 这行配置的名字，比如 FX_Vaporize_Default
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FXRowName = NAME_None;

	// 先用 Cascade 粒子占位
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UParticleSystem> CascadeFX = nullptr;

	// 生成位置偏移
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SpawnOffset = FVector::ZeroVector;

	// 生成缩放
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SpawnScale = FVector(1.f, 1.f, 1.f);

	// 是否附着到目标
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAttachToTarget = false;

	// 大于 0 时，到了这个时间会强制停止并销毁特效，避免循环粒子常驻
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
	float AutoDestroyDelaySeconds = 0.f;

	// 调试用
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DebugText;
};
