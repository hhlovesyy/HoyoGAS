#include "GAS/SurvivorAbilitySystemComponent.h"

#include "Core/SurvivorArenaLog.h"

void USurvivorAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	/*
	*在多人联机（比如你的《土豆兄弟》项目）中，GAS 的核心组件是 AbilitySystemComponent (简称 ASC)。
	因为它是和 PlayerState（名片）以及 Character（肉体）分离的，所以它刚被创建出来的时候，就像是一个失忆的打工人，它根本不知道自己属于谁，也不知道该把技能特效生在谁身上。
	这时候，就需要由 InitAbilityActorInfo 来进行“角色认亲”。这个函数接收两个极其重要的参数：
		OwnerActor（所有者/后台大脑）： 拥有这个 ASC 的真正主人。在你的项目里，就是 PlayerState。它负责管钱（金币）、管属性（攻击力、攻速）和网络同步。
		AvatarActor（化身/前台肉体）： 技能和特效在游戏世界里的物理载体。在你的项目里，就是 Character。它负责跑位、播放动作（Montage）、碰撞检测。
	// “报告长官！我的后台数据和网络同步听 PlayerState(Owner) 的；
	//  我的物理位移、技能特效、动作播放全都实现在 Character(Avatar) 身上！” 
	*/
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	UE_LOG(
		LogSurvivorArena,
		Log,
		TEXT("Survivor ASC initialized. Owner=%s Avatar=%s"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InAvatarActor));
}
