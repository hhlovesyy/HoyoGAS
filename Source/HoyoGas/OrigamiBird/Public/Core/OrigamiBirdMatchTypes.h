#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "OrigamiBirdMatchTypes.generated.h"

//1.三消游戏支持的棋盘内容类型
UENUM(BlueprintType)
enum class EOrigamiBirdTileType: uint8
{
	None,
	RedFruit,
	BlueFruit,
	GreenFruit,
	YellowFruit,
	PurpleFruit,
};

// 方块策划表：定义“某一种格子”的表现和基础规则。
// JSON 导入 DataTable 时，每一行代表一种方块，比如红色水果、蓝色水果、冰块、石头。
USTRUCT(BlueprintType)
struct HOYOGAS_API FOrigamiBirdTileDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	// 对应棋盘格子的类型。普通三消水果用 RedFruit/BlueFruit 等，障碍物可以先扩展 enum 再配表。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	EOrigamiBirdTileType TileType = EOrigamiBirdTileType::None;

	// 策划/调试用名字，UI 可以直接显示。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FText DisplayName;

	// UI 图标。JSON 里填 Texture2D 资源路径，导入 DataTable 后由 UI 或 VM LoadSynchronous。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TSoftObjectPtr<UTexture2D> Icon;

	// 调试 UI 可以先用颜色块代替正式图标。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FLinearColor DebugColor = FLinearColor::White;

	// 是否能参与三消匹配。普通水果为 true，石头/空洞等障碍物为 false。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	bool bCanMatch = true;

	// 是否会在消除后受重力下落。普通水果为 true，固定障碍物/冰块底座通常为 false。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	bool bCanFall = true;

	// 是否允许玩家拖动交换。普通水果为 true，固定障碍物通常为 false。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	bool bCanSwap = true;

	// 单个方块被消除时的基础分。普通玩法可都配 10，特殊方块可更高。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 ScoreValue = 10;
};

//2.游戏进行的状态
UENUM(BlueprintType)
enum class EOrigamiBirdMatchPhase : uint8
{
	None,
	WaitingInput,
	Resolving,
	GameEnded
};

//3.一个格子的数据结构，需要保存对应三消东西的类型，位置，ID，是否选中
USTRUCT(BlueprintType)
struct HOYOGAS_API FOrigamiBirdTile
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 TileId = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FIntPoint BoardPosition = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	EOrigamiBirdTileType TileType = EOrigamiBirdTileType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	bool bIsSelected = false;
	
};

//4.游戏开始的StartParams，设定一些参数
USTRUCT(BlueprintType)
struct HOYOGAS_API FOrigamiBirdMatchStartParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 BoardWidth = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 BoardHeight = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 MoveLimit = 25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 TargetScore = 3000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 RandomSeed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TArray<EOrigamiBirdTileType> AvailableTileTypes; //本局允许的格子类型

	// 本局使用的方块定义。正式项目通常来自 TileDefinition DataTable，这里放进 StartParams 便于测试和复用。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TArray<FOrigamiBirdTileDefinitionRow> TileDefinitions;
};

// 关卡策划表：定义“一局怎么玩”。
// JSON 导入 DataTable 时，每一行代表一关，比如 Level_001。
USTRUCT(BlueprintType)
struct HOYOGAS_API FOrigamiBirdLevelDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	// 关卡显示名，只负责 UI 和调试日志，不参与规则判断。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FText DisplayName;

	// 棋盘宽度。折纸小鸟对对碰类似 7x7，但这里允许策划调整。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird", meta = (ClampMin = "3", ClampMax = "12"))
	int32 BoardWidth = 7;

	// 棋盘高度。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird", meta = (ClampMin = "3", ClampMax = "12"))
	int32 BoardHeight = 7;

	// 限制步数。每次有效交换扣 1。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird", meta = (ClampMin = "1"))
	int32 MoveLimit = 25;

	// 目标分数。达到目标后进入 GameEnded。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird", meta = (ClampMin = "1"))
	int32 TargetScore = 3000;

	// 随机种子。固定种子方便复现 bug；上线可以由关卡系统传入动态种子。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 RandomSeed = 12345;

	// 本关会自然生成的方块类型。这里通常只放普通可消除水果，不放固定障碍物。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TArray<EOrigamiBirdTileType> AvailableTileTypes;

	FOrigamiBirdMatchStartParams ToStartParams() const
	{
		FOrigamiBirdMatchStartParams Params;
		Params.BoardWidth = BoardWidth;
		Params.BoardHeight = BoardHeight;
		Params.MoveLimit = MoveLimit;
		Params.TargetScore = TargetScore;
		Params.RandomSeed = RandomSeed;
		Params.AvailableTileTypes = AvailableTileTypes;
		return Params;
	}
};

UENUM(BlueprintType)
enum class EOrigamiBirdMatchCommandType : uint8
{
	None,
	SelectTile,
	SwapTiles,
	Restart
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FOrigamiBirdMatchCommand
{
	GENERATED_BODY()
	
	//使用Command的好处在于，或许命令不来自用户，可能是AI，或者是自动化跑测脚本
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	EOrigamiBirdMatchCommandType CommandType = EOrigamiBirdMatchCommandType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FIntPoint From = FIntPoint(INDEX_NONE, INDEX_NONE);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FIntPoint To = FIntPoint(INDEX_NONE, INDEX_NONE);
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FOrigamiBirdBoardSnapshot
{
	GENERATED_BODY()
	
	//UI不应该直接去GameObject里面拿数据，而是应该有一个Snapshot，从这里面拿数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 BoardWidth = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 BoardHeight = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TArray<FOrigamiBirdTile> Tiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 Score = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 MovesRemaining = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 MaxCombo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 RemovedTileCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	EOrigamiBirdMatchPhase Phase = EOrigamiBirdMatchPhase::None;
};
