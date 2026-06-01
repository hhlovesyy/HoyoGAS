#include "GAS/SurvivorAbilitySet.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Core/SurvivorArenaLog.h"
#include "GameplayEffect.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

void USurvivorAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* ASC, UObject* SourceObject) const
{
	if (!ASC)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("USurvivorAbilitySet::GiveToAbilitySystem skipped because ASC is null."));
		return;
	}

	const UWorld* World = ASC->GetWorld();
	const AActor* OwnerActor = ASC->GetOwnerActor();
	const bool bCanGrantInStandalone = World == nullptr || World->GetNetMode() == NM_Standalone;
	const bool bHasAuthority = OwnerActor && OwnerActor->HasAuthority();
	if (!bHasAuthority && !bCanGrantInStandalone)
	{
		//这是 GAS 开发的铁律：赋予技能（GiveAbility）和赋予永久被动属性（ApplyGameplayEffectToSelf）的操作，必须且只能在服务器（Server）上进行！
		//如果客户端自己悄悄调用了这个函数，给自己发了一个“攻击力一刀999”的大礼包，这段代码会瞬间将它拦截（!bHasAuthority 为真，直接 return），完美防外挂。
		UE_LOG(
			LogSurvivorArena,
			Verbose,
			TEXT("USurvivorAbilitySet::GiveToAbilitySystem skipped on non-authority ASC. Owner=%s AbilitySet=%s"),
			*GetNameSafe(OwnerActor),
			*GetNameSafe(this));
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : AbilitiesToGrant)
	{
		if (!AbilityClass)
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("AbilitySet %s contains a null ability entry."), *GetNameSafe(this));
			continue;
		}
		/*
		*在 GAS 的设计哲学里，你不能直接 New 一个 GameplayAbility（技能）对象塞给玩家。因为技能对象是很占内存的，而且牵涉到极其复杂的网络状态同步。

		你需要给 ASC 塞一个 FGameplayAbilitySpec（技能规格说明书）。
		你可以把 FGameplayAbilitySpec 理解为给玩家发了一本“技能证书”。这段代码参数的硬核拆解如下：

		AbilityClass：这是技能的具体蓝图类（比如 BP_GA_火球术）。引擎知道玩家学会了这个技能，但此时并没有真正实例化。只有当玩家按下按键要释放技能时，引擎才会根据这个类，动态生成出真正的技能对象去执行逻辑（极大地节省了内存）。

		1 (Level)：技能等级。比如你可以赋予一个 1 级的火球术，以后升级了，底层可以根据这个等级去查表算出不同的伤害。

		INDEX_NONE (InputID)：输入绑定 ID。如果你想让这个技能固定绑定到某个按键（比如填 0 代表左键，1 代表右键），这里可以填数字。但在现代基于 Enhanced Input 的架构下，通常这里填 INDEX_NONE（相当于 -1），然后在控制器里通过 AbilityId 或 Tag 来灵活触发。

		SourceObject：技能来源。这非常关键。比如这个技能是因为玩家装备了“火焰法杖”才获得的，那么这里就可以传“法杖”的指针。以后如果玩家把法杖脱了，你就可以通过查找这个 SourceObject，把对应的技能从玩家身上精准地剥离掉。
		 */
		ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, SourceObject));
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : PassiveEffectsToApply)
	{
		if (!EffectClass)
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("AbilitySet %s contains a null passive effect entry."), *GetNameSafe(this));
			continue;
		}
		
		/*
		*【核心玄机】：获取类默认对象（CDO, Class Default Object）。
		这是虚幻引擎最精华的底层机制之一。在 GAS 里，GameplayEffect 纯粹就是一堆数据的集合（扣多少血、加什么 Tag）。如果每个玩家被砍一刀，都去内存里 New 一个全新的 GE 对象，那 100 个怪同时互砍，内存和垃圾回收（GC）直接就爆炸了。
		所以，我们永远不实例化 GE，而是直接通过 GetDefaultObject 拿到这个类的“出厂母版（CDO）”。CDO 在游戏启动时就加载在内存里了，它是一份只读的静态数据，极度轻量且高效。
		 */
		const UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
		if (!EffectCDO)
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("AbilitySet %s could not resolve passive effect CDO for %s."), *GetNameSafe(this), *GetNameSafe(*EffectClass));
			continue;
		}
		/*
		*【解释】：创建上下文句柄（Context）。
		既然 GE 是公用的母版（不包含当前是谁打谁的信息），那么“谁给我的这个 Buff？”、“是在哪里命中的？”这些具体的动态信息存在哪？答案就是 Context。你可以把 Context 理解为包裹着 Buff 的“快递单号”。
		 */
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		if (SourceObject)
		{
			EffectContext.AddSourceObject(SourceObject); //【解释】：在“快递单号”上写下发件人的名字。和前面技能的 SourceObject 一样，记录下是谁（比如是一件装备，还是一个关卡 Buff）给了我这个被动效果，方便追溯和后续卸载。
		}
		/*
		*【解释】：最终执行。
		对当前玩家自己的 ASC（ToSelf）应用这个被动效果。
		传入了三个极其精准的参数：
			EffectCDO：效果数据是什么？（加 100 血的母版）
			1.0f：效果是几级？（1级）
			EffectContext：谁发来的快递？（包含来源信息的上下文）
		至此，这个被动 Buff 才真正生效，底层会自动去调用我们上一回讲到的 AttributeSet 里的 PostGameplayEffectExecute，让角色的血量上限立刻增加 100。
		 */
		ASC->ApplyGameplayEffectToSelf(EffectCDO, 1.0f, EffectContext);
	}

	UE_LOG(LogSurvivorArena, Log, TEXT("Applied ability set %s to ASC owner %s."), *GetNameSafe(this), *GetNameSafe(OwnerActor));
}
