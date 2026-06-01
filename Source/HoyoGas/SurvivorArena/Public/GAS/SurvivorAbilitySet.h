#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SurvivorAbilitySet.generated.h"

/*
 *“玩家开局或者捡到一个道具时，我怎么把对应的技能和属性发给他？”
 *如果按照新手写法，可能会在 Character 的 C++ 里写一堆：ASC->GiveAbility(Fireball); ASC->GiveAbility(Dash);
 *这会导致代码极其臃肿，而且策划无法配置。USurvivorAbilitySet (DataAsset) 就是用来解决这个问题的“大礼包”：
*你可以让策划在编辑器里创建一个 DataAsset（数据资产）文件，比如叫 DA_Hero_Potato：
在 AbilitiesToGrant 数组里，填入：冲刺技能、普通攻击技能。
在 PassiveEffectsToApply 数组里，填入：初始血量+100 的 GE、初始移速+50 的 GE。
当游戏开始（结合我们上一回看的代码），你只需要调用 GiveToAbilitySystem，这个大礼包就会被一次性拆开，全部塞进玩家的 ASC 里！
 */

class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;

UCLASS(BlueprintType)
class HOYOGAS_API USurvivorAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TArray<TSubclassOf<UGameplayEffect>> PassiveEffectsToApply;

	UFUNCTION(BlueprintCallable, Category = "SurvivorArena")
	void GiveToAbilitySystem(UAbilitySystemComponent* ASC, UObject* SourceObject = nullptr) const;
};
