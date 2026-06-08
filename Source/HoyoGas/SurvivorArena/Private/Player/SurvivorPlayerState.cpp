#include "Player/SurvivorPlayerState.h"

#include "Cards/SurvivorCardLoadoutComponent.h"
#include "Core/SurvivorArenaLog.h"
#include "GAS/SurvivorAbilitySystemComponent.h"
#include "GAS/SurvivorAttributeSet.h"
#include "Net/UnrealNetwork.h"

ASurvivorPlayerState::ASurvivorPlayerState()
{
	SetNetUpdateFrequency(100.0f); //PlayerState 默认的同步频率非常低（通常只有 1Hz 左右，因为传统游戏中它只用来同步计分板的杀敌数）。但在 GAS 架构下，PlayerState 承载了角色的血量、蓝量、技能 CD 等高频战斗数据。把它拉高到 100.0f（每秒同步100次），确保了动作游戏中极低的输入延迟和状态同步延迟。这是一个非常有经验的设置。
	//服务器每秒钟会去检查 100 次（每 10 毫秒一次）。如果发现你的经验值、血量或者 Buff 发生了改变，立刻打包发车。注意，是有变化才发，不是死板地每秒强行发 100 个空包，所以不用太担心带宽爆炸。
	AbilitySystemComponent = CreateDefaultSubobject<USurvivorAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	//Mixed的模式：对于玩家自己（Owner）：服务器会把详细的 GE 数据全部同步给你，因为你的 UI 需要显示具体的 Buff 剩余时间、层数和详细数值。
	//对于其他看你的玩家（Simulated Proxies）：服务器只会把你的 GameplayTags 和 GameplayCues 同步给他们。别人不需要知道你的攻击力 Buff 还有几秒结束，只需要看到你身上有发光的特效即可。
	//结论：Mixed 是玩家控制角色（PlayerState）的唯一正确选择。
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<USurvivorAttributeSet>(TEXT("AttributeSet"));
	CardLoadoutComponent = CreateDefaultSubobject<USurvivorCardLoadoutComponent>(TEXT("CardLoadoutComponent"));
}

void ASurvivorPlayerState::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogSurvivorArena, Log, TEXT("SurvivorPlayerState BeginPlay: %s"), *GetNameSafe(this));
}

void ASurvivorPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	//这段代码是虚幻引擎 C++ 中用于网络同步（Replication）的“死规定（Boilerplate）”。
	/*
	它的工作流是这样的：
		头文件声明：你在 .h 文件里写了 UPROPERTY(Replicated) 声明了 SurvivorLevel，意思是告诉引擎：“这个变量以后是要通过网络同步的”。
		CPP 注册：但是光说不行，UE 引擎底层是一个极其严谨的 C++ 框架。只要你加了 Replicated，你就必须重写 GetLifetimeReplicatedProps 这个函数。
		DOREPLIFETIME 宏：在这个函数里，你用 DOREPLIFETIME 把类名和变量名填进去。这相当于去引擎的网络大管家那里登记备案：“长官，一旦服务器上 ASurvivorPlayerState 身上的 SurvivorLevel 发生了变化，请立刻把它打包，通过网线发送给所有的客户端！”
	 */
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASurvivorPlayerState, SurvivorLevel);
}

UAbilitySystemComponent* ASurvivorPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

USurvivorAbilitySystemComponent* ASurvivorPlayerState::GetSurvivorAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

const USurvivorAttributeSet* ASurvivorPlayerState::GetSurvivorAttributeSet() const
{
	return AttributeSet;
}

USurvivorCardLoadoutComponent* ASurvivorPlayerState::GetCardLoadoutComponent() const
{
	return CardLoadoutComponent;
}

int32 ASurvivorPlayerState::GetSurvivorLevel() const
{
	return SurvivorLevel;
}

void ASurvivorPlayerState::SetSurvivorLevel(int32 NewLevel)
{
	SurvivorLevel = FMath::Max(1, NewLevel);
}

float ASurvivorPlayerState::GetCurrentExperience() const
{
	return AttributeSet ? AttributeSet->GetExperience() : 0.0f;
}
