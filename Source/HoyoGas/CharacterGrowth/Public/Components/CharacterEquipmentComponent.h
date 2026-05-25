#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/HoyoRelicTypes.h"
#include "CharacterEquipmentComponent.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHoyoCharacterEquipmentChangedSignature, FName, CharacterId);

// 角色装备业务组件：管理玩家拥有的遗器实例，以及每个角色当前穿戴的 6 个槽位。
// 这一层不直接等同于 GAS；它负责背包/穿戴/套装统计，后续再把汇总结果转换成 GE。
UCLASS(ClassGroup = (CharacterGrowth), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class HOYOGAS_API UCharacterEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterEquipmentComponent();

	// 把一件遗器实例加入玩家背包。实例的槽位/套装/稀有度会从遗器定义表校验并回填。
	UFUNCTION(BlueprintCallable, Category = "CharacterGrowth|Relic")
	bool AddRelicInstance(FHoyoRelicInstance RelicInstance);

	// 从背包移除一件遗器；如果它已被穿戴，会先从对应角色身上卸下。
	UFUNCTION(BlueprintCallable, Category = "CharacterGrowth|Relic")
	bool RemoveRelicInstance(FGuid RelicInstanceId);

	// 给角色穿戴一件遗器。若目标槽位已有遗器，会自动替换；若该遗器在其他角色身上，会先从旧角色卸下。
	UFUNCTION(BlueprintCallable, Category = "CharacterGrowth|Relic")
	bool EquipRelic(FName CharacterId, FGuid RelicInstanceId);

	// 卸下角色某个槽位的遗器。
	UFUNCTION(BlueprintCallable, Category = "CharacterGrowth|Relic")
	bool UnequipRelic(FName CharacterId, EHoyoRelicSlot Slot);

	// 获取玩家背包里所有遗器实例。
	UFUNCTION(BlueprintPure, Category = "CharacterGrowth|Relic")
	void GetAllRelics(TArray<FHoyoRelicInstance>& OutRelics) const;

	// 按实例 ID 查找一件遗器。
	UFUNCTION(BlueprintPure, Category = "CharacterGrowth|Relic")
	bool GetRelicInstance(FGuid RelicInstanceId, FHoyoRelicInstance& OutRelic) const;

	// 获取某个角色穿戴的所有遗器实例。
	UFUNCTION(BlueprintPure, Category = "CharacterGrowth|Relic")
	void GetEquippedRelics(FName CharacterId, TArray<FHoyoRelicInstance>& OutRelics) const;

	// 获取某个角色指定槽位上的遗器实例。
	UFUNCTION(BlueprintPure, Category = "CharacterGrowth|Relic")
	bool GetEquippedRelicInSlot(FName CharacterId, EHoyoRelicSlot Slot, FHoyoRelicInstance& OutRelic) const;

	// 计算角色当前激活了哪些二件套/四件套。
	UFUNCTION(BlueprintPure, Category = "CharacterGrowth|Relic")
	void GetActiveRelicSetBonuses(FName CharacterId, TArray<FHoyoRelicSetActivation>& OutActivations) const;

	// 开发测试入口：从遗器策划表按 6 个槽位生成测试实例并装备到指定角色。
	// 只用于验证 DataTable -> 装备组件 -> UI 的链路，不应该作为正式掉落/背包逻辑使用。
	// 可在 Project Settings 的 Hoyo Character Growth 里关闭；Shipping 包会强制返回 false。
	UFUNCTION(BlueprintCallable, Category = "CharacterGrowth|Relic|Development")
	bool GrantRelicDevelopmentLoadout(FName CharacterId, bool bReplaceExisting = false);

	// 当前是否允许使用遗器开发测试入口。UI 可用它决定是否显示或启用测试按钮。
	UFUNCTION(BlueprintPure, Category = "CharacterGrowth|Relic|Development")
	bool AreRelicDevelopmentActionsEnabled() const;

	// 从策划表读取遗器定义。
	UFUNCTION(BlueprintPure, Category = "CharacterGrowth|Relic|Data")
	bool GetRelicDefinition(FName RelicDefinitionId, FHoyoRelicDefinitionRow& OutDefinition) const;

	// 从策划表读取套装定义。
	UFUNCTION(BlueprintPure, Category = "CharacterGrowth|Relic|Data")
	bool GetRelicSetDefinition(FName SetId, FHoyoRelicSetDefinitionRow& OutDefinition) const;

	// 从策划表读取词条定义。
	UFUNCTION(BlueprintPure, Category = "CharacterGrowth|Relic|Data")
	bool GetRelicAffixDefinition(FName AffixId, FHoyoRelicAffixDefinitionRow& OutDefinition) const;

	UPROPERTY(BlueprintAssignable, Category = "CharacterGrowth|Relic")
	FHoyoCharacterEquipmentChangedSignature OnEquipmentChanged;

private:
	int32 FindRelicInventoryIndex(FGuid RelicInstanceId) const;
	int32 FindLoadoutIndex(FName CharacterId) const;
	FHoyoCharacterRelicLoadout& FindOrAddLoadout(FName CharacterId);
	FHoyoEquippedRelicSlot& FindOrAddEquippedSlot(FHoyoCharacterRelicLoadout& Loadout, EHoyoRelicSlot Slot);
	void ClearRelicFromLoadouts(FGuid RelicInstanceId, FName& OutPreviousCharacterId);
	bool NormalizeRelicFromDefinition(FHoyoRelicInstance& RelicInstance) const;
	bool BuildDevelopmentRelicInstance(const FHoyoRelicDefinitionRow& RelicDefinition, FHoyoRelicInstance& OutRelicInstance) const;

	UDataTable* LoadRelicDefinitionTable() const;
	UDataTable* LoadRelicSetDefinitionTable() const;
	UDataTable* LoadRelicAffixDefinitionTable() const;

	// 玩家拥有的遗器实例。正式接存档时，这个数组就是需要序列化的核心数据之一。指的是玩家拥有的所有遗器
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterGrowth|Relic", meta = (AllowPrivateAccess = "true"))
	TArray<FHoyoRelicInstance> RelicInventory;

	// 每个角色当前穿戴的 6 个遗器槽位。正式接存档时，它和 RelicInventory 一起落盘。每个角色穿了哪六件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterGrowth|Relic", meta = (AllowPrivateAccess = "true"))
	TArray<FHoyoCharacterRelicLoadout> CharacterRelicLoadouts;
};
