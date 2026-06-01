#pragma once

#include "CoreMinimal.h"
#include "Data/SurvivorArenaTypes.h"
#include "GameFramework/GameStateBase.h"
#include "SurvivorGameState.generated.h"

UCLASS()
class HOYOGAS_API ASurvivorGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ASurvivorGameState();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "SurvivorArena")
	ESurvivorRunState GetRunState() const;

	UFUNCTION(BlueprintPure, Category = "SurvivorArena")
	float GetElapsedTime() const;

	UFUNCTION(BlueprintPure, Category = "SurvivorArena")
	float GetRemainingTime() const;

	UFUNCTION(BlueprintPure, Category = "SurvivorArena")
	int32 GetCurrentWaveIndex() const;

	UFUNCTION(BlueprintPure, Category = "SurvivorArena")
	int32 GetKillCount() const;

	void SetRunState(ESurvivorRunState NewRunState);
	void SetElapsedTime(float NewElapsedTime);
	void SetRemainingTime(float NewRemainingTime);
	void SetCurrentWaveIndex(int32 NewWaveIndex);
	void SetKillCount(int32 NewKillCount);
	void ResetRunMetrics();

protected:
	/*
	普通的属性同步（比如倒计时 RemainingTime），客户端只是默默地接受数值的改变。
	但是加了 ReplicatedUsing，事情就发生了质变。这叫做 RepNotify（同步回调）。
	它的工作流是这样的：
		服务器端：把 RunState 从 Combat（战斗中）改成了 Shop（商店/结算）。
		网络底层：把这个变化发送给客户端。
		客户端接收到后：引擎不仅会修改客户端上的 RunState 变量，还会自动强制调用一遍 OnRep_RunState 这个函数！
	实战意义： 这简直是做 UI 和表现层的神器！你不需要在客户端每帧去判断状态有没有变，你只需要在 OnRep_RunState 这个函数里写：
		“如果变成 Shop 状态，播放胜利音效，并呼叫 UI 子系统弹出商店界面。”
		“如果变成 Combat 状态，播放激昂的 BGM，关闭商店界面。”
		这种基于事件驱动的响应机制，极大地节省了性能。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_RunState, Category = "SurvivorArena")
	ESurvivorRunState RunState = ESurvivorRunState::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "SurvivorArena")
	float ElapsedTime = 0.0f; //已用时间

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "SurvivorArena")
	float RemainingTime = 0.0f; //剩余时间

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "SurvivorArena")
	int32 CurrentWaveIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "SurvivorArena")
	int32 KillCount = 0;

	UFUNCTION()
	void OnRep_RunState(ESurvivorRunState PreviousRunState);
};
