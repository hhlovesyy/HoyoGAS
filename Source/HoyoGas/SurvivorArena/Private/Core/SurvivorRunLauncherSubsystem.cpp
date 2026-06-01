#include "Core/SurvivorRunLauncherSubsystem.h"

#include "Core/SurvivorArenaLog.h"
#include "Core/SurvivorArenaSettings.h"
#include "Kismet/GameplayStatics.h"

/*
*为了解决跨关卡传参的痛点，原作者使用了 UGameInstanceSubsystem（游戏实例子系统）。
在 UE5 中，各种 Subsystem 的生命周期是不同的：
UWorldSubsystem：生命周期跟着当前关卡走，换关卡它就会死。
UGameInstanceSubsystem：它的生命周期贯穿整个游戏进程。 只要游戏没有完全退出（进程没关），它在内存里就永远活着，绝对不会因为切换关卡而被销毁。
 */
bool USurvivorRunLauncherSubsystem::LaunchSurvivorRun(const FSurvivorRunStartConfig& Config)
{
	//当玩家在主菜单点下“开始”：
	const USurvivorArenaSettings* Settings = GetDefault<USurvivorArenaSettings>();
	if (!Settings || Settings->SurvivorArenaMapName.IsNone())
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("LaunchSurvivorRun failed because SurvivorArenaMapName is not configured."));
		return false;
	}

	PendingStartConfig = BuildResolvedConfig(Config); //1. 把玩家选好的配置稳妥地存进自己的变量里
	bHasPendingStartConfig = true; // 2. 举起小红旗：表示“有尚未处理的起跑配置”

	UE_LOG(
		LogSurvivorArena,
		Log,
		TEXT("Launching Survivor run. Map=%s CharacterId=%s LevelId=%s RandomSeed=%d FixedSeed=%s"),
		*Settings->SurvivorArenaMapName.ToString(),
		*PendingStartConfig.CharacterId.ToString(),
		*PendingStartConfig.LevelId.ToString(),
		PendingStartConfig.RandomSeed,
		PendingStartConfig.bUseFixedSeed ? TEXT("true") : TEXT("false"));

	UGameplayStatics::OpenLevel(this, Settings->SurvivorArenaMapName); //3. 换地图！这个新地图打开之后会唤起GameMode的Begin函数，很巧妙的设计，于是GameMode就回来访问SubSystem了
	return true;
}

bool USurvivorRunLauncherSubsystem::ConsumePendingStartConfig(FSurvivorRunStartConfig& OutConfig)
{
	if (!bHasPendingStartConfig)
	{
		return false;
	}

	OutConfig = PendingStartConfig;
	PendingStartConfig = FSurvivorRunStartConfig();
	bHasPendingStartConfig = false;

	UE_LOG(
		LogSurvivorArena,
		Log,
		TEXT("Consumed pending Survivor run config. CharacterId=%s LevelId=%s RandomSeed=%d FixedSeed=%s"),
		*OutConfig.CharacterId.ToString(),
		*OutConfig.LevelId.ToString(),
		OutConfig.RandomSeed,
		OutConfig.bUseFixedSeed ? TEXT("true") : TEXT("false"));
	return true;
}

FSurvivorRunStartConfig USurvivorRunLauncherSubsystem::BuildResolvedConfig(const FSurvivorRunStartConfig& InConfig) const
{
	FSurvivorRunStartConfig ResolvedConfig = InConfig;
	const USurvivorArenaSettings* Settings = GetDefault<USurvivorArenaSettings>();

	if (ResolvedConfig.CharacterId.IsNone() && Settings)
	{
		ResolvedConfig.CharacterId = Settings->DefaultCharacterId;
	}

	if (ResolvedConfig.LevelId.IsNone() && Settings)
	{
		ResolvedConfig.LevelId = Settings->DefaultLevelId;
	}

	if (!ResolvedConfig.bUseFixedSeed && ResolvedConfig.RandomSeed == 0)
	{
		ResolvedConfig.RandomSeed = FMath::Rand();
	}

	return ResolvedConfig;
}
