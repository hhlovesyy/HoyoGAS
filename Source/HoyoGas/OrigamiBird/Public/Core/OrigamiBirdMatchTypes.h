#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Templates/SubclassOf.h"
#include "OrigamiBirdMatchTypes.generated.h"

class UOrigamiBirdPropEffect;

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

// 道具的业务类型。这里描述“使用后要对棋盘做什么”。
// 第一版先用 enum 驱动 GameObject 内部逻辑；等道具复杂到 switch 不好维护时，再拆成策略类或 UObject。
UENUM(BlueprintType)
enum class EOrigamiBirdPropType : uint8
{
	None,

	// 移除一个指定格子，然后触发下落和补充。
	RemoveSingleTile,

	// 把一个指定格子替换成 ReplacementTileType。
	ReplaceTileType,

	// 交换两整列。
	SwapColumns,

	// 把一列复制到另一列。
	CopyColumn
};

// 道具需要玩家选择什么目标。
// UI 会根据它决定点击道具后进入哪种选择模式。
UENUM(BlueprintType)
enum class EOrigamiBirdPropTargetType : uint8
{
	None,

	// 选择一个棋盘格子，例如锤子、替换水果。
	SingleTile,

	// 选择一列，例如清除一列。
	SingleColumn,

	// 选择两列，例如交换两列、复制一列到另一列。
	TwoColumns
};

// 道具策略参数：一条 Key/Value 配置。
// 这样同一个 EffectClass 可以通过不同参数复用，例如 ReplaceTileEffect 配 TileType=RedFruit 或 TileType=BlueFruit。
USTRUCT(BlueprintType)
struct HOYOGAS_API FOrigamiBirdPropEffectParam
{
	GENERATED_BODY()

	// 参数名。建议使用稳定英文名，例如 TileType、ResolveAfterUse、Count。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FName Key = NAME_None;

	// 参数值。统一用字符串保存，再由策略类按 bool/int/float/name/enum 读取。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FString Value;
};

// 道具策划表：每一行定义一种可使用道具。
// RowName 就是道具 ID，例如 Prop_RemoveSingle、Prop_ReplaceRed、Prop_SwapColumns。
USTRUCT(BlueprintType)
struct HOYOGAS_API FOrigamiBirdPropDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	// UI 显示名，例如“敲掉一个水果”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FText DisplayName;

	// UI 描述文本，例如“选择一个格子，将其移除并补充新水果”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FText Description;

	// 道具图标。后面右侧道具 ListView 会显示它。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TSoftObjectPtr<UTexture2D> Icon;

	// 道具实际效果类型。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	EOrigamiBirdPropType PropType = EOrigamiBirdPropType::None;

	// 使用这个道具时，UI 需要玩家选择的目标类型。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	EOrigamiBirdPropTargetType TargetType = EOrigamiBirdPropTargetType::None;

	// 道具效果策略类。正式扩展时优先通过这个类执行效果，而不是把所有道具写进一个 switch。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TSubclassOf<UOrigamiBirdPropEffect> EffectClass;

	// 道具效果参数。由 EffectClass 自己解释，避免每个小变体都新建一个道具类。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TArray<FOrigamiBirdPropEffectParam> EffectParams;

	// 开发期/关卡默认给予数量。后面做活动存档时，运行时数量会来自存档或关卡奖励。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird", meta = (ClampMin = "0"))
	int32 InitialCount = 0;

	// 是否可以堆叠。不能堆叠的道具在一局里最多持有 1 个。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	bool bStackable = true;

	// 最大堆叠数量。bStackable=false 时会按 1 处理。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird", meta = (ClampMin = "1"))
	int32 MaxStackCount = 99;

	// 使用后是否立即触发三消连锁解算。替换、移除通常为 true；纯交换列也可以按策划决定。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	bool bResolveAfterUse = true;

	// ReplaceTileType 专用：把目标格子替换成哪一种水果。
	// 其他道具类型会忽略这个字段。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	EOrigamiBirdTileType ReplacementTileType = EOrigamiBirdTileType::None;
};

// 一局游戏里某个道具的运行时数量。
// PropId 对应 PropDefinition DataTable 的 RowName。
USTRUCT(BlueprintType)
struct HOYOGAS_API FOrigamiBirdPropStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FName PropId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird", meta = (ClampMin = "0"))
	int32 Count = 0;
};

// 一次使用道具的请求。
// UI/VM 负责根据 TargetType 收集目标，然后把目标放进这个结构传给玩法逻辑。
USTRUCT(BlueprintType)
struct HOYOGAS_API FOrigamiBirdPropUseRequest
{
	GENERATED_BODY()

	// 要使用的道具 ID，对应道具 DataTable 的 RowName。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FName PropId = NAME_None;

	// 格子目标。SingleTile 类道具通常只填 1 个；范围/多选类道具可以填多个。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TArray<FIntPoint> TargetPositions;

	// 列目标。SingleColumn 填 1 个，TwoColumns 填 2 个。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TArray<int32> TargetColumns;

	// 行目标。后面如果做清除一行、复制一行，可以直接复用这个字段。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TArray<int32> TargetRows;
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

// 一次交换解算里的表现步骤类型。
// 逻辑可以瞬间算完，但 UI 动画需要按这些步骤依次播放。
UENUM(BlueprintType)
enum class EOrigamiBirdResolveStepType : uint8
{
	None,

	// 两个相邻方块交换位置。
	Swap,

	// 找到一组或多组可消除匹配，通常先播放高亮。
	Match,

	// 匹配方块被移除，通常播放缩放/淡出。
	Remove,

	// 老方块从上方掉到下方。
	Fall,

	// 新方块从棋盘上方或原地生成。
	Spawn,

	// 分数、连击、步数等数值变化。
	Score,

	// 动画播完后，用最终 Snapshot 强制校准 UI。
	FinalSnapshot
};

// 一个方块从 FromPosition 移动到 ToPosition。
// Swap 和 Fall 动画都可以用这个结构。
USTRUCT(BlueprintType)
struct HOYOGAS_API FOrigamiBirdTileTransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 TileId = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	EOrigamiBirdTileType TileType = EOrigamiBirdTileType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FIntPoint FromPosition = FIntPoint(INDEX_NONE, INDEX_NONE);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FIntPoint ToPosition = FIntPoint(INDEX_NONE, INDEX_NONE);
};

// 一次解算中的一个表现步骤。
// 不同 StepType 会使用不同字段：Swap/Fall 用 TileTransitions，Match/Remove/Spawn 用 AffectedTiles。
USTRUCT(BlueprintType)
struct HOYOGAS_API FOrigamiBirdResolveStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	EOrigamiBirdResolveStepType StepType = EOrigamiBirdResolveStepType::None;

	// 交换或下落时，记录每个方块从哪里移动到哪里。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TArray<FOrigamiBirdTileTransition> TileTransitions;

	// 匹配、消除、生成时，记录受影响的方块。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TArray<FOrigamiBirdTile> AffectedTiles;

	// 有些表现只关心格子位置，比如高亮一组匹配位置。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TArray<FIntPoint> AffectedPositions;

	// 本步骤增加的分数。只有 Score 步骤通常会填。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 ScoreDelta = 0;

	// 当前连锁序号：第一次消除为 1，二次连锁为 2。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 ComboIndex = 0;

	// 本步骤移除的方块数量。Remove/Score 步骤会用到。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 RemovedTileCount = 0;

	// 这个步骤播放完之后的棋盘快照。第一版可以不填；后面做复杂校准时再用。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FOrigamiBirdBoardSnapshot SnapshotAfterStep;
};

// 一次玩家移动的完整解算结果。
// UI 后续会按 ResolveSteps 顺序播放动画，最后用 FinalSnapshot 校准最终显示。
USTRUCT(BlueprintType)
struct HOYOGAS_API FOrigamiBirdMoveResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	bool bAccepted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FIntPoint From = FIntPoint(INDEX_NONE, INDEX_NONE);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FIntPoint To = FIntPoint(INDEX_NONE, INDEX_NONE);

	// 如果被拒绝，记录原因 ID。比如 NotAdjacent、NoMatch、InvalidPhase。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FName FailureReasonId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FOrigamiBirdBoardSnapshot InitialSnapshot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FOrigamiBirdBoardSnapshot FinalSnapshot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TArray<FOrigamiBirdResolveStep> ResolveSteps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 TotalScoreDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 UsedMoveDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 MaxCombo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 RemovedTileCount = 0;
};

// 一次道具使用的完整结果。
// 结构和 MoveResult 相似：玩法逻辑先瞬间算完，UI 再按 ResolveSteps 播放表现。
USTRUCT(BlueprintType)
struct HOYOGAS_API FOrigamiBirdPropUseResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	bool bAccepted = false;

	// 失败原因 ID。比如 PropNotFound、NotEnoughTarget、InvalidColumn。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FName FailureReasonId = NAME_None;

	// 使用的道具 ID，方便 UI/日志/事件系统知道是哪一个道具触发。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FName PropId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FOrigamiBirdBoardSnapshot InitialSnapshot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	FOrigamiBirdBoardSnapshot FinalSnapshot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	TArray<FOrigamiBirdResolveStep> ResolveSteps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 TotalScoreDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrigamiBird")
	int32 RemovedTileCount = 0;
};
