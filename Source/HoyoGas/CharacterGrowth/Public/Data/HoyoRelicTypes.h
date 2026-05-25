#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Engine/DataTable.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "HoyoRelicTypes.generated.h"

class UGameplayEffect;
class UTexture2D;

// 星穹铁道式遗器槽位：4 个洞穴遗器 + 2 个位面饰品。
UENUM(BlueprintType)
enum class EHoyoRelicSlot : uint8
{
	// 头部：固定主词条通常是生命值。
	Head,

	// 手部：固定主词条通常是攻击力。
	Hands,

	// 躯干：主词条可能是暴击率、暴击伤害、治疗量、效果命中等。
	Body,

	// 脚部：主词条可能是速度、攻击力百分比、防御力百分比等。
	Feet,

	// 位面球：主词条通常承载属性伤害、生命/攻击/防御百分比等。
	PlanarSphere,

	// 连结绳：主词条通常承载能量恢复效率、击破特攻、生命/攻击/防御百分比等。
	LinkRope
};

// 遗器数值的显示/计算类型：用于区分固定值和百分比。
UENUM(BlueprintType)
enum class EHoyoRelicStatValueType : uint8
{
	// 固定值，例如生命值 +120、攻击力 +38。
	Flat,

	// 百分比，例如攻击力 +12%、暴击率 +5.8%。
	Percent
};

// 一条可转换成 GAS Modifier 的遗器属性加成。
USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoRelicStatModifier
{
	GENERATED_BODY()

	// 词条或套装效果的业务标签，例如 Relic.Stat.CritRate，用于 UI 分类和日志。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	FGameplayTag StatTag;

	// 这条遗器属性最终修改哪个 GAS Attribute，例如 AttackPower、CritRate、EnergyRecharge。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	FGameplayAttribute TargetAttribute;

	// GAS 修改方式：通常固定值/百分比面板加成用 Additive，复杂公式后续再扩展。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	TEnumAsByte<EGameplayModOp::Type> ModifierOp = EGameplayModOp::Additive;

	// 数值类型：Flat 表示固定值，Percent 表示百分比，UI 会用它决定显示格式。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	EHoyoRelicStatValueType ValueType = EHoyoRelicStatValueType::Flat;

	// 实际数值。百分比建议用小数存储，例如 0.058 表示 5.8%。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	float Value = 0.0f;
};

// 词条定义表：描述“暴击率、攻击力百分比、速度”等词条本身是什么。
USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoRelicAffixDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	// 词条 ID，例如 CritRate、AttackPercent、Speed。实例数据只保存这个 ID。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FName AffixId = NAME_None;

	// UI 显示名，例如“暴击率”“攻击力”“能量恢复效率”。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FText DisplayName;

	// 词条标签，用于筛选/分类/条件判断，例如 Relic.Affix.CritRate。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FGameplayTag AffixTag;

	// 这个词条最终影响的 GAS 属性。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FGameplayAttribute TargetAttribute;

	// 这个词条转换为 GameplayEffect Modifier 时使用的修改方式。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	TEnumAsByte<EGameplayModOp::Type> ModifierOp = EGameplayModOp::Additive;

	// 固定值还是百分比；例如速度是 Flat，暴击率是 Percent。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	EHoyoRelicStatValueType ValueType = EHoyoRelicStatValueType::Flat;
};

// 单条词条实例：某件遗器上的“暴击率 +5.8%”这种落地数据。
USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoRelicAffixInstance
{
	GENERATED_BODY()

	// 指向词条定义表里的 AffixId。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	FName AffixId = NAME_None;

	// 当前词条数值。百分比同样用小数存储，例如 0.058 表示 5.8%。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	float Value = 0.0f;

	// 副词条被强化命中的次数；主词条通常不使用这个字段。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	int32 UpgradeCount = 0;
};

// 套装效果定义：二件套、四件套分别是两条记录。
USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoRelicSetBonusDefinition
{
	GENERATED_BODY()

	// 激活需要同套装几件；星穹铁道常见是 2 或 4。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic", meta = (ClampMin = "1"))
	int32 RequiredPieceCount = 2;

	// UI 描述，例如“二件套：攻击力提高 12%”。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic", meta = (MultiLine = "true"))
	FText Description;

	// 纯数值套装效果可直接写成 modifier，后续汇总进装备 GameplayEffect。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	TArray<FHoyoRelicStatModifier> Modifiers;

	// 复杂套装效果可配置一个 Infinite GE，例如条件增伤、战斗内叠层等。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	TSoftClassPtr<UGameplayEffect> GameplayEffectClass;
};

// 套装定义表：描述“野穗伴行的快枪手”这种套装及它的二件套/四件套。
USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoRelicSetDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	// 套装 ID，例如 Musketeer、Messenger。遗器定义会引用它。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FName SetId = NAME_None;

	// UI 显示名，例如“野穗伴行的快枪手”。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FText DisplayName;

	// 套装标签，例如 Relic.Set.Musketeer，用于统计和条件判断。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FGameplayTag SetTag;

	// 套装总描述，可用于详情页顶部说明。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic", meta = (MultiLine = "true"))
	FText Description;

	// 套装激活效果列表，通常包含 RequiredPieceCount=2 和 4 两条。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	TArray<FHoyoRelicSetBonusDefinition> Bonuses;
};

// 遗器基础定义表：描述某个可掉落模板属于哪个套装、哪个槽位、基础名字和图标。
USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoRelicDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	// 遗器模板 ID，例如 Musketeer_Head。实例数据会引用它。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FName RelicDefinitionId = NAME_None;

	// 所属套装 ID，用于计算二件套/四件套。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FName SetId = NAME_None;

	// 槽位：头、手、躯干、脚、位面球、连结绳。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	EHoyoRelicSlot Slot = EHoyoRelicSlot::Head;

	// UI 显示名，例如“快枪手的野穗毡帽”。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FText DisplayName;

	// 星级上限或默认星级，例如 5 星遗器。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic", meta = (ClampMin = "1", ClampMax = "5"))
	int32 Rarity = 5;

	// 图标资源，用于圣遗物槽位和详情页显示。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	TSoftObjectPtr<UTexture2D> Icon;

	// 该槽位允许出现的主词条 ID 列表；头/手可以只放固定主词条。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	TArray<FName> AllowedMainAffixIds;
};

// 一件遗器的存档实例：真正属于玩家背包的随机词条装备。
USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoRelicInstance
{
	GENERATED_BODY()

	// 实例唯一 ID。两件同名同套装遗器也会有不同 InstanceId。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	FGuid InstanceId;

	// 指向遗器基础定义表的 RelicDefinitionId。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	FName RelicDefinitionId = NAME_None;

	// 冗余保存套装 ID，方便存档读取和套装统计；可由定义表校验。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	FName SetId = NAME_None;

	// 当前实例所在槽位。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	EHoyoRelicSlot Slot = EHoyoRelicSlot::Head;

	// 当前强化等级，例如 +0 到 +15。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic", meta = (ClampMin = "0"))
	int32 Level = 0;

	// 当前星级，例如 4 星或 5 星。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic", meta = (ClampMin = "1", ClampMax = "5"))
	int32 Rarity = 5;

	// 主词条，例如躯干的“暴击率 +32.4%”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	FHoyoRelicAffixInstance MainAffix;

	// 副词条列表，例如“速度 +2”“暴击伤害 +11.6%”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	TArray<FHoyoRelicAffixInstance> SubAffixes;

	// 当前装备在哪个角色身上；None 表示在背包中未装备。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	FName EquippedCharacterId = NAME_None;

	// 是否锁定，避免强化/分解时误操作。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	bool bLocked = false;
};

// 单个装备槽位的存档映射：角色某个槽位穿了哪件遗器。
USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoEquippedRelicSlot
{
	GENERATED_BODY()

	// 装备槽位。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	EHoyoRelicSlot Slot = EHoyoRelicSlot::Head;

	// 装在该槽位上的遗器实例 ID；无效 Guid 表示空槽。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	FGuid RelicInstanceId;
};

// 某个角色当前穿戴的 6 件遗器。
USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoCharacterRelicLoadout
{
	GENERATED_BODY()

	// 角色 ID，例如 Trailblazer_Stelle。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	FName CharacterId = NAME_None;

	// 6 个槽位对应的装备实例。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic")
	TArray<FHoyoEquippedRelicSlot> EquippedSlots;
};

// 当前角色已激活的套装统计结果：这是运行时/UI 派生数据，不需要直接进策划表。
USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoRelicSetActivation
{
	GENERATED_BODY()

	// 套装 ID，例如 Musketeer。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FName SetId = NAME_None;

	// 当前角色身上穿了这个套装的几件。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	int32 EquippedPieceCount = 0;

	// 已经激活的套装档位，例如穿 4 件时一般包含 2 和 4。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	TArray<int32> ActiveBonusPieceCounts;
};
