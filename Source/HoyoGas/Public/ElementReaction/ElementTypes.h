#pragma once

#include "CoreMinimal.h"
#include "ElementTypes.generated.h"

UENUM(BlueprintType)
enum class EGenshinElementType : uint8
{
	None	UMETA(DisplayName = "None"),
	Pyro	UMETA(DisplayName = "Pyro"),
	Hydro	UMETA(DisplayName = "Hydro"),
	Cryo	UMETA(DisplayName = "Cryo")
};

UENUM(BlueprintType)
enum class EReactionType : uint8
{
	None		UMETA(DisplayName = "None"),
	Vaporize	UMETA(DisplayName = "Vaporize"),
	Melt		UMETA(DisplayName = "Melt"),
	Frozen		UMETA(DisplayName = "Frozen")
};

USTRUCT(BlueprintType)
struct FCombatStats
{
	GENERATED_BODY()

	// // 攻击力：先把它当成这一下伤害的主要来源
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// float Attack = 100.f;

	// 元素精通：影响蒸发/融化这类增幅反应
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ElementalMastery = 0.f;

	// 角色等级：这一步先存着，后面做剧变反应会更重要
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CharacterLevel = 1;
};

USTRUCT(BlueprintType)
struct FElementAura
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGenshinElementType Element = EGenshinElementType::None;

	// 当前剩余元素量。现在把它当作唯一真实状态，反应消耗和自然衰减都直接作用在这里。
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.f;
	
	// 当前这层 Aura 的参考最大元素量，用于 UI 百分比，同时和 MaxDuration 一起编码衰减率。
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue = 1.f;

	// 剩余持续时间。它不再独立驱动逻辑，而是由 Value 和衰减率实时推导。
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.f;
	
	// 对应 MaxValue 的参考时长。MaxDuration / MaxValue 代表这层 Aura 的每单位衰减时间。
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDuration = 10.f;
};

USTRUCT(BlueprintType)
struct FElementHitEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGenshinElementType IncomingElement = EGenshinElementType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage = 100.f;
	
	// 这次攻击来源的战斗属性
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCombatStats SourceStats;
};

UENUM(BlueprintType)
enum class EDamageTextStyleType : uint8
{
	Normal      UMETA(DisplayName = "Normal"),
	Vaporize    UMETA(DisplayName = "Vaporize"),
	Melt        UMETA(DisplayName = "Melt"),
	Frozen      UMETA(DisplayName = "Frozen")
};

USTRUCT(BlueprintType)
struct FReactionResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EReactionType Reaction = EReactionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FinalDamage = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bApplyFrozen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FElementAura NewAura;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ReactionDisplayText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ReactionColor = FLinearColor::White;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDamageTextStyleType DamageTextStyle = EDamageTextStyleType::Normal;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseFreezeMaterial = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FreezeAmount = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReactionFXRowName = NAME_None;
};

USTRUCT(BlueprintType)
struct FReactionVisualConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ReactionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ReactionColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDamageTextStyleType DamageTextAnimType = EDamageTextStyleType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseFreezeMaterial = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FreezeAmount = 0.f;
};

UENUM(BlueprintType)
enum class EAuraRefreshPolicy : uint8
{
	// 只刷新持续时间
	RefreshDuration UMETA(DisplayName = "RefreshDuration"),

	// 整个 Aura 全部重置
	OverrideAll UMETA(DisplayName = "OverrideAll"),

	// 如果已经是同元素，就忽略这次刷新
	IgnoreIfSameElement UMETA(DisplayName = "IgnoreIfSameElement")
};



