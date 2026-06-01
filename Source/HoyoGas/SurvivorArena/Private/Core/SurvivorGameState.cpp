#include "Core/SurvivorGameState.h"

#include "Core/SurvivorArenaLog.h"
#include "Net/UnrealNetwork.h"

//GameState 就是“全局大喇叭/公告板”： 它的宿命就是由服务器（Server）来修改数据，然后引擎底层会自动把这些数据同步给所有连在房间里的客户端（Client）。而客户端拿到这些数据，最主要的目的就是驱动全局 UI 的显示。
//血量是个人的，所以放在 PlayerState。
ASurvivorGameState::ASurvivorGameState()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASurvivorGameState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASurvivorGameState, RunState);
	DOREPLIFETIME(ASurvivorGameState, ElapsedTime);
	DOREPLIFETIME(ASurvivorGameState, RemainingTime);
	DOREPLIFETIME(ASurvivorGameState, CurrentWaveIndex);
	DOREPLIFETIME(ASurvivorGameState, KillCount);
}

ESurvivorRunState ASurvivorGameState::GetRunState() const
{
	return RunState;
}

float ASurvivorGameState::GetElapsedTime() const
{
	return ElapsedTime;
}

float ASurvivorGameState::GetRemainingTime() const
{
	return RemainingTime;
}

int32 ASurvivorGameState::GetCurrentWaveIndex() const
{
	return CurrentWaveIndex;
}

int32 ASurvivorGameState::GetKillCount() const
{
	return KillCount;
}

void ASurvivorGameState::SetRunState(ESurvivorRunState NewRunState)
{
	//这些 Set 函数里面，作者没有写任何网络鉴权的判定，而是直接修改了变量。
	//这是因为在虚幻的网络架构中，有一个隐性契约：GameState 里的这些 Setter 函数，绝对不应该由客户端去调用。它们只能由 GameMode（裁判）来调用。
	//如果客户端（比如玩家的电脑）因为开挂，强行调用了 SetCurrentWaveIndex(100)，这个数据绝对不会同步给其他人，也不会同步给服务器，只会在他自己的电脑上自嗨，因为 UE 底层严格禁止客户端向服务器同步 GameState 的属性。
	if (RunState == NewRunState)
	{
		return;
	}

	const ESurvivorRunState PreviousRunState = RunState;
	RunState = NewRunState;

	UE_LOG(LogSurvivorArena, Log, TEXT("Survivor run state changed: %d -> %d"), static_cast<int32>(PreviousRunState), static_cast<int32>(RunState));
}

void ASurvivorGameState::SetElapsedTime(float NewElapsedTime)
{
	ElapsedTime = FMath::Max(0.0f, NewElapsedTime);
}

void ASurvivorGameState::SetRemainingTime(float NewRemainingTime)
{
	RemainingTime = FMath::Max(0.0f, NewRemainingTime);
}

void ASurvivorGameState::SetCurrentWaveIndex(int32 NewWaveIndex)
{
	CurrentWaveIndex = NewWaveIndex;
}

void ASurvivorGameState::SetKillCount(int32 NewKillCount)
{
	KillCount = FMath::Max(0, NewKillCount);
}

void ASurvivorGameState::ResetRunMetrics()
{
	ElapsedTime = 0.0f;
	RemainingTime = 0.0f;
	CurrentWaveIndex = INDEX_NONE;
	KillCount = 0;
}

void ASurvivorGameState::OnRep_RunState(ESurvivorRunState PreviousRunState)
{
	//在虚幻引擎的网络底层逻辑中，当你为 ReplicatedUsing（RepNotify）函数添加一个与该变量同类型的参数时，引擎的执行顺序是这样的：
	//先更新局部变量： 客户端收到服务器发来的网络包后，引擎在底层会先把客户端本地的 RunState 变量更新为服务器传来的最新值。
	//触发回调并塞入旧值： 引擎紧接着调用你写的 OnRep_RunState 函数。并且它非常贴心地把刚刚被覆盖掉的旧值，自动塞进 PreviousRunState 这个参数里传给你。
	UE_LOG(LogSurvivorArena, Log, TEXT("Survivor run state replicated: %d -> %d"), static_cast<int32>(PreviousRunState), static_cast<int32>(RunState));
}
