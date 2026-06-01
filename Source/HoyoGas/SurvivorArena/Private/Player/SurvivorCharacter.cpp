#include "Player/SurvivorCharacter.h"

#include "Core/SurvivorArenaLog.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/SurvivorPlayerState.h"
#include "GAS/SurvivorAbilitySystemComponent.h"

ASurvivorCharacter::ASurvivorCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationYaw = false; //角色不会强制跟着控制器的镜头方向转动。
	//俯视角射击游戏的基础移动设置
	if (UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement())
	{
		CharacterMovementComponent->bOrientRotationToMovement = true; //角色会朝着它移动的方向转身（比如按下 S 键，角色会转身往屏幕下方跑，而不是背对着屏幕倒退）。
		CharacterMovementComponent->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	}
}

void ASurvivorCharacter::PossessedBy(AController* NewController)
{
	//在 GAS 架构中，你需要调用 InitAbilityActorInfo 来告诉系统：“谁是这个能力的主人（Owner），谁是这个能力的物理载体（Avatar）”。
	//但在联网游戏中，服务器和客户端得知“角色准备就绪”的时间点是完全不同的。
	//PossessedBy函数仅在服务器（Server）上调用。触发时机：当服务器生成了这个角色（Character），并且分配了一个控制器（PlayerController 或 AIController）去“附身（Possess）”它的时候。为什么在这里初始化 GAS：在服务器上，当 PossessedBy 被触发时，角色的 PlayerState 通常已经被安全地创建并分配好了。此时立刻调用 InitAbilityActorInfo()，可以确保服务器端的 GAS 系统第一时间就绪，准备好处理后续的技能释放和伤害计算。
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
}

void ASurvivorCharacter::OnRep_PlayerState()
{
	//调用环境：仅在客户端（Client）上调用。
	//触发时机：当服务器把角色的 PlayerState 指针通过网络同步（Replicate）给客户端，且客户端成功接收到时触发。
	//为什么不在客户端的 BeginPlay 里初始化？：这是新手最容易犯的错误。在 UE 的网络同步中，数据的到达顺序是不保证的。客户端可能先收到了 Character，但此时 PlayerState 还在网络传输半路上。如果你在客户端的 BeginPlay 里去获取 PlayerState，极大概率会拿到一个空指针（Nullptr）导致崩溃。
	//正确的做法：引擎提供了 OnRep_PlayerState 这个回调。当这个函数触发时，引擎是在明确地告诉你：“嘿，客户端，这个角色的 PlayerState 终于从服务器传过来了！” 这时才是客户端调用 InitAbilityActorInfo() 唯一的、绝对安全的时机。
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
}

UAbilitySystemComponent* ASurvivorCharacter::GetAbilitySystemComponent() const
{
	return CachedAbilitySystemComponent;
}

void ASurvivorCharacter::InitAbilityActorInfo()
{
	ASurvivorPlayerState* SurvivorPlayerState = GetPlayerState<ASurvivorPlayerState>();
	if (!SurvivorPlayerState)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("SurvivorCharacter %s could not initialize ASC because PlayerState is null."), *GetNameSafe(this));
		CachedAbilitySystemComponent = nullptr;
		return;
	}

	CachedAbilitySystemComponent = SurvivorPlayerState->GetSurvivorAbilitySystemComponent();
	if (!CachedAbilitySystemComponent)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("SurvivorCharacter %s could not initialize ASC because ASC is null."), *GetNameSafe(this));
		return;
	}

	CachedAbilitySystemComponent->InitAbilityActorInfo(SurvivorPlayerState, this);
	UE_LOG(LogSurvivorArena, Log, TEXT("SurvivorCharacter initialized ASC ActorInfo. Character=%s PlayerState=%s"), *GetNameSafe(this), *GetNameSafe(SurvivorPlayerState));
}
