#include "Core/SurvivorGameMode.h"

#include "Core/SurvivorArenaLog.h"
#include "Core/SurvivorArenaSettings.h"
#include "Core/SurvivorGameState.h"
#include "Core/SurvivorRunLauncherSubsystem.h"
#include "Engine/World.h"
#include "Player/SurvivorCharacter.h"
#include "Player/SurvivorPlayerController.h"
#include "Player/SurvivorPlayerState.h"

/*
*在多人联机游戏中，GameMode 资产和实例只存在于服务器端。任何客户端的电脑内存里，GetGameMode() 返回的永远是 nullptr。
这意味着：
	所有核心的规则决定（如：游戏何时开始、何时结束、胜负判定、刷怪控制）都必须写在 GameMode 里。
	客户端绝对无法通过开挂篡改 GameMode 的逻辑，因为他们根本拿不到这个对象。
 */

ASurvivorGameMode::ASurvivorGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	GameStateClass = ASurvivorGameState::StaticClass();
	PlayerControllerClass = ASurvivorPlayerController::StaticClass();
	PlayerStateClass = ASurvivorPlayerState::StaticClass();
	DefaultPawnClass = ASurvivorCharacter::StaticClass();
}

void ASurvivorGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogSurvivorArena, Log, TEXT("SurvivorGameMode BeginPlay: %s"), *GetNameSafe(this));

	//在《土豆兄弟》这类游戏中，玩家通常在主菜单关卡选择角色、武器和地图，然后点击“开始游戏”跳转到战斗关卡。
	//痛点：关卡一切换，旧地图的所有 Actor 全被销毁，选好的角色 ID 怎么带过去？
	//解法：这段代码利用了生命周期贯穿全局的 GameInstanceSubsystem。主菜单把配置存在子系统里，新关卡的 GameMode 在 BeginPlay 时通过 ConsumePendingStartConfig 把配置“吃”出来。这是极其标准且解耦的商业游戏数据流传方式。
	FSurvivorRunStartConfig StartConfig;
	bool bHasPendingConfig = false;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USurvivorRunLauncherSubsystem* LauncherSubsystem = GameInstance->GetSubsystem<USurvivorRunLauncherSubsystem>())
		{
			bHasPendingConfig = LauncherSubsystem->ConsumePendingStartConfig(StartConfig);
		}
	}

	if (!bHasPendingConfig)
	{
		const USurvivorArenaSettings* Settings = GetDefault<USurvivorArenaSettings>();
		StartConfig.CharacterId = Settings ? Settings->DefaultCharacterId : NAME_None;
		StartConfig.LevelId = Settings ? Settings->DefaultLevelId : NAME_None;
	}

	StartSurvivorRun(StartConfig);
}

void ASurvivorGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ASurvivorGameState* SurvivorGameState = GetGameState<ASurvivorGameState>();
	if (!SurvivorGameState)
	{
		return;
	}
	
	//为什么不让 GameState 自己在 Tick 里给自己算时间？
	//因为 GameMode 才是时间的掌控者。如果暂停游戏、或者进入了局内商店阶段，GameMode 可以通过改变 CurrentRunState 随时让计时器停下来。GameMode 算好正确的时间后，再通过 SetElapsedTime 塞给 GameState，触发属性同步给全场玩家。
	if (CurrentRunState == ESurvivorRunState::InRun || CurrentRunState == ESurvivorRunState::Preparing)
	{
		SurvivorGameState->SetElapsedTime(GetWorld() ? GetWorld()->GetTimeSeconds() - RunStartTimeSeconds : 0.0f);
	}
}

void ASurvivorGameMode::StartSurvivorRun(const FSurvivorRunStartConfig& Config)
{
	CurrentRunConfig = Config;
	RunStartTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	UE_LOG(
		LogSurvivorArena,
		Log,
		TEXT("Starting Survivor run. CharacterId=%s LevelId=%s RandomSeed=%d FixedSeed=%s"),
		*CurrentRunConfig.CharacterId.ToString(),
		*CurrentRunConfig.LevelId.ToString(),
		CurrentRunConfig.RandomSeed,
		CurrentRunConfig.bUseFixedSeed ? TEXT("true") : TEXT("false"));

	if (ASurvivorGameState* SurvivorGameState = GetGameState<ASurvivorGameState>())
	{
		SurvivorGameState->ResetRunMetrics();
	}

	SetRunState(ESurvivorRunState::Preparing);
	SetRunState(ESurvivorRunState::InRun);
}

void ASurvivorGameMode::EndSurvivorRun(bool bVictory)
{
	SetRunState(bVictory ? ESurvivorRunState::Victory : ESurvivorRunState::Defeat);
	UE_LOG(LogSurvivorArena, Log, TEXT("Survivor run ended. Victory=%s"), bVictory ? TEXT("true") : TEXT("false"));
}

void ASurvivorGameMode::SetRunState(ESurvivorRunState NewRunState)
{
	if (CurrentRunState == NewRunState)
	{
		return;
	}

	const ESurvivorRunState PreviousRunState = CurrentRunState;
	CurrentRunState = NewRunState;

	if (ASurvivorGameState* SurvivorGameState = GetGameState<ASurvivorGameState>())
	{
		SurvivorGameState->SetRunState(NewRunState);
	}

	UE_LOG(LogSurvivorArena, Log, TEXT("SurvivorGameMode state transition: %d -> %d"), static_cast<int32>(PreviousRunState), static_cast<int32>(CurrentRunState));
}
