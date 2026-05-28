# OrigamiBird Gameplay Refactor Plan

本文档用于记录 OrigamiBird 三消玩法的当前结构、重构边界和分阶段计划。目标是让后续重构始终围绕职责边界推进，避免在实现过程中偏离方向。

## 1. 重构目标

本次重构不是重写玩法，而是在尽量保持现有行为的前提下改善结构。

核心目标：

- 玩法规则可以单独阅读、单独推理、后续便于测试。
- 一局游戏状态和棋盘算法分离。
- 玩法解算事实和 UI 表现时间线分离。
- UI 只消费快照、结果和表现时间线，不直接理解过多规则细节。
- Subsystem 只负责生命周期和配置入口，不承载玩法规则。
- 每一步重构都足够小，便于回退和人工检查。

暂不作为本轮目标：

- 不重写整个 OrigamiBird 模块。
- 不引入 GAS。
- 不大规模重做 UI。
- 不在第一阶段追求完整 QA 自动化覆盖。
- 不把玩法解算顺序交给策划配置。

## 2. 当前结构地图

当前主要文件和职责如下。

### Core

- `OrigamiBirdMatchTypes.h`
  - 定义玩法公共数据结构。
  - 包含 Tile、关卡参数、棋盘快照、移动结果、道具结果、ResolveStep 等。

- `OrigamiBirdMatchGameObject.h/.cpp`
  - 当前玩法核心。
  - 负责棋盘数据、初始化、交换、匹配扫描、消除、下落、补充、计分、步数、结算、道具库存、道具效果落地、ResolveStep 生成和事件广播。
  - 这是当前职责最重、后续最需要拆分的类。

- `OrigamiBirdPropEffect.h/.cpp`
  - 道具效果策略基类。
  - 负责从配置读取参数，并提供 `Execute` 扩展点。

- `OrigamiBirdRemoveSingleTilePropEffect.*`
- `OrigamiBirdBoardEditPropEffects.*`
  - 具体道具效果策略。
  - 当前策略类通常校验目标，然后调用 `UOrigamiBirdMatchGameObject::ApplyProp...`。

### Application

- `OrigamiBirdMatchSubsystem.h/.cpp`
  - 当前作为 LocalPlayerSubsystem。
  - 负责加载关卡、Tile、道具 DataTable。
  - 负责创建和持有 ActiveMatch。
  - 对外提供开始一局、结束一局、使用道具等入口。

### ViewModel

- `VM_OrigamiBirdMatchScreen.h/.cpp`
  - 负责主界面状态。
  - 当前也承担 Tile 选择、相邻判断、交换命令、道具目标收集、道具选择状态、状态文案等逻辑。

- `VM_OrigamiBirdPropEntry.h/.cpp`
  - 负责道具列表项的显示数据。

### Widgets

- `OrigamiBirdMatchScreen.h/.cpp`
  - 页面级 UI。
  - 连接按钮、棋盘点击、道具列表选择和 ViewModel。

- `OrigamiBirdBoardWidget.h/.cpp`
  - 棋盘表现层。
  - 根据 Snapshot 构建棋盘。
  - 根据 `FOrigamiBirdMoveResult` / `FOrigamiBirdPropUseResult` 中的 `ResolveSteps` 顺序播放动画。

- `OrigamiBirdTileVisualWidget.h/.cpp`
  - 单个 Tile 的显示和点击。

## 3. 当前主要调用链

### 3.1 启动一局

```text
MatchScreen::InitializeViewModel
-> VM_OrigamiBirdMatchScreen::Initialize
-> StartDefaultMatch
-> OrigamiBirdMatchSubsystem::StartDefaultDevelopmentMatch
-> StartMatchByLevelId
-> NewObject<UOrigamiBirdMatchGameObject>
-> InitializeFromLevelDefinition
-> Initialize
-> GenerateInitialBoard
-> BroadcastBoardChanged
```

### 3.2 玩家点击交换

```text
TileVisualWidget::HandleHitButtonClicked
-> BoardWidget::HandleTileVisualClicked
-> MatchScreen::HandleTileClicked
-> VM_OrigamiBirdMatchScreen::SelectOrSwapTile
-> UOrigamiBirdMatchGameObject::TrySwapTilesWithResult
-> FindAllMatches
-> ResolveCurrentMatchesIntoSteps
-> BroadcastBoardChanged
-> MatchScreen calls BoardWidget::PlayMoveResult
-> BoardWidget plays ResolveSteps
```

### 3.3 道具使用

```text
PropList selection
-> MatchScreen::HandlePropSelectionChanged
-> VM stores selected prop state

Board click
-> MatchScreen::HandleTileClicked
-> VM_OrigamiBirdMatchScreen::TryUseSelectedPropOnTile
-> OrigamiBirdMatchSubsystem::UsePropOnActiveMatch
-> UOrigamiBirdMatchGameObject::UsePropWithResult
-> UOrigamiBirdPropEffect::Execute
-> UOrigamiBirdMatchGameObject::ApplyProp...
-> optional ResolveCurrentMatchesIntoSteps
-> BoardWidget::PlayPropUseResult
```

## 4. 现有设计优点

- 核心规则和 UI 表现已经有初步分离。
- `Snapshot` 让 UI 不需要直接读取内部棋盘数组。
- `MoveResult` / `PropUseResult` 为动画播放提供了完整上下文。
- 道具通过 `EffectClass` 扩展，方向上比巨大 switch 更可扩展。
- 交换、匹配、消除、下落、补充分布在少数核心函数中，仍然能追踪。

## 5. 现有结构问题

### 5.1 `UOrigamiBirdMatchGameObject` 职责过重

它当前同时负责：

- 棋盘数据存储。
- 坐标和索引访问。
- 初始棋盘生成。
- 匹配扫描。
- 交换合法性。
- 消除、下落、补充。
- 分数、步数、结算。
- 道具库存。
- 道具效果落地。
- UI ResolveStep 生成。
- 事件广播。

这个类会随着玩法扩展持续膨胀，是本轮重构的主要目标。

### 5.2 玩法事实和表现步骤耦合

当前 `FOrigamiBirdResolveStep` 同时描述：

- 玩法中发生了什么。
- UI 应该按什么顺序播放动画。

这会导致两个问题：

- 读玩法逻辑时必须理解 UI 表现步骤。
- UI 动画天然被数组顺序串行化，不利于并行动效和错峰动效。

例如分数飞字、计分器跳动、消除动画、下落动画未来可能需要部分重叠，但当前顺序 Step 难以表达。

### 5.3 ViewModel 承担了较多输入状态机

`VM_OrigamiBirdMatchScreen` 现在不仅是 UI 状态，还处理：

- 第一次点击选中。
- 第二次点击交换。
- 相邻判断。
- 道具选择。
- 道具目标收集。
- 两列选择状态。

这些逻辑可以后续拆成更明确的小状态对象，但优先级低于核心规则拆分。

### 5.4 DataTable 配置和运行时职责边界不够明确

当前道具配置里同时存在 `PropType`、`EffectClass`、`EffectParams`、`ReplacementTileType` 等字段。后续需要明确：

- 哪些字段是运行时真正使用的。
- 哪些字段只是调试或旧设计遗留。
- 避免多个字段表达同一个决策，导致维护者不知道哪个是权威入口。

## 6. 目标结构

建议逐步演进为以下结构。

```text
OrigamiBird/Core
  OrigamiBirdMatchTypes
  OrigamiBirdBoardState
  OrigamiBirdBoardResolver
  OrigamiBirdMatchGameObject
  OrigamiBirdPropEffect

OrigamiBird/Application
  OrigamiBirdMatchSubsystem

OrigamiBird/Presentation
  OrigamiBirdResolvePresentationTypes
  OrigamiBirdResolvePresentationCompiler
  OrigamiBirdBoardWidget
  OrigamiBirdMatchScreen
  VM_OrigamiBirdMatchScreen
```

### 6.1 `FOrigamiBirdBoardState`

负责棋盘数据和基础访问。

建议职责：

- 保存 `BoardWidth`、`BoardHeight`、`Tiles`。
- `ToIndex`
- `IsInsideBoard`
- `GetTile`
- `SwapTileData`
- `RemoveTiles`
- `MakeSnapshotTiles` 或类似辅助函数。

不负责：

- 分数。
- 步数。
- Phase。
- UI 动画步骤。
- DataTable 加载。
- 道具库存。

### 6.2 `FOrigamiBirdBoardResolver`

负责纯棋盘算法。

建议职责：

- `GenerateInitialBoard`
- `WouldCreateMatchAt`
- `FindAllMatches`
- `CollapseAndRefill`
- `ResolveCurrentMatches`

可以接收：

- `FOrigamiBirdBoardState&`
- 随机流。
- 可生成 Tile 类型。
- TileDefinition 规则访问接口或轻量规则表。

输出应该是玩法事实，不应该直接输出 UI 动画步骤。

### 6.3 `UOrigamiBirdMatchGameObject`

负责一局游戏的运行时状态和对外入口。

建议保留职责：

- `Initialize`
- `TrySwapTilesWithResult`
- `UsePropWithResult`
- 分数、步数、最大 Combo、移除数量。
- Phase。
- 道具库存。
- 广播事件。
- 构造最终的 Action Result。

不应该长期负责：

- 具体棋盘扫描算法。
- UI 动画时间线。
- 所有道具效果细节。

### 6.4 Presentation Compiler

新增表现编译层，把玩法事实转换成 UI 可播放时间线。

```text
玩法结果
-> ResolveCycles
-> PresentationCompiler
-> PresentationTimeline
-> BoardWidget 播放
```

这个层允许：

- 策划配置动画节奏。
- 同一时间播放多个表现事件。
- 避免玩法层知道 UI 动画时长。

## 7. Resolve 重构方向

当前：

```text
GameObject
-> FOrigamiBirdResolveStep[]
-> BoardWidget sequential playback
```

目标：

```text
GameObject / BoardResolver
-> FOrigamiBirdResolveCycle[]
-> PresentationCompiler
-> FOrigamiBirdPresentationTimeline
-> BoardWidget timeline playback
```

### 7.1 玩法事实：ResolveCycle

示意结构：

```cpp
USTRUCT(BlueprintType)
struct FOrigamiBirdResolveCycle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ComboIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FIntPoint> MatchPositions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FOrigamiBirdTile> RemovedTiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ScoreDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FOrigamiBirdTileTransition> FallTransitions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FOrigamiBirdTile> SpawnedTiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FOrigamiBirdBoardSnapshot SnapshotAfterCycle;
};
```

### 7.2 表现事件：PresentationEvent

示意结构：

```cpp
UENUM(BlueprintType)
enum class EOrigamiBirdPresentationEventType : uint8
{
	None,
	Swap,
	MatchHighlight,
	Remove,
	ScorePopup,
	ScoreCounter,
	Fall,
	Spawn,
	BoardSync
};

USTRUCT(BlueprintType)
struct FOrigamiBirdPresentationEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EOrigamiBirdPresentationEventType Type = EOrigamiBirdPresentationEventType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FOrigamiBirdTileTransition> Transitions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FOrigamiBirdTile> Tiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FIntPoint> Positions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ScoreDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ComboIndex = 0;
};
```

这样可以表达：

```text
0.00 Swap
0.16 MatchHighlight
0.26 Remove
0.26 ScorePopup
0.34 ScoreCounter
0.40 Fall
0.48 Spawn
```

而不是强制每个 Step 串行执行。

## 8. 策划配置边界

建议开放给策划：

- Swap 动画时长。
- Match 高亮时长。
- Remove 动画时长。
- ScorePopup 相对 Remove 的延迟。
- Fall 相对 Remove 的延迟。
- Fall 动画时长。
- Spawn 相对 Fall 的延迟或重叠。
- Spawn 动画时长。
- Combo 对表现节奏的倍率或额外特效等级。

不建议开放给策划：

- 匹配扫描顺序。
- 消除、下落、补充的玩法顺序。
- 是否先计分再移除这种核心规则时序。
- 棋盘状态修改的执行顺序。

原因：玩法规则时序如果表驱动，会显著增加复现和调试成本。表现节奏可以配置，玩法规则应保持代码固定且可测试。

## 9. 分阶段重构计划

### Phase 0：建立地图和边界

目标：不改逻辑，先建立共享认知。

任务：

- 新增本文档。
- 后续可补充一张当前调用链图。
- 标记每个类当前职责和目标职责。
- 确认第一轮重构不动 UI 表现、不动道具行为。

验收：

- 文档能解释当前结构。
- 文档能指导下一步拆分。

### Phase 1：可读性整理

目标：不改行为，只降低阅读成本。

任务：

- 修复乱码注释或改成清晰中文/英文。
- 给 `TrySwapTilesWithResult`、`ResolveCurrentMatchesIntoSteps`、`CollapseAndRefill` 增加简短流程注释。
- 集中定义常见 FailureReason 名称，减少散落字符串。
- 标注暂时不重构的旧字段，避免误读。

验收：

- 编译通过。
- 交换、道具、棋盘显示行为不变。

### Phase 2：拆出 BoardState

目标：把棋盘数据访问从 `GameObject` 中拿出来。

任务：

- 新增 `FOrigamiBirdBoardState`。
- 移动基础函数：
  - `ToIndex`
  - `IsInsideBoard`
  - `GetTile`
  - `SwapTileData`
  - `RemoveTiles`
- `GameObject` 继续保留对外接口。
- 不改变 ResolveStep 结构。
- 不改变 UI。

验收：

- 编译通过。
- 原有点击交换流程仍可运行。
- `GameObject` 代码体积明显下降。

### Phase 3：拆出 BoardResolver

目标：把纯棋盘算法从 `GameObject` 中拿出来。

任务：

- 新增 `FOrigamiBirdBoardResolver`。
- 移动：
  - `GenerateInitialBoard`
  - `WouldCreateMatchAt`
  - `FindAllMatches`
  - `CollapseAndRefill`
  - `ResolveCurrentMatches`
- 暂时仍可输出旧的 `ResolveSteps`，减少 UI 改动风险。

验收：

- 编译通过。
- 有效交换、无效交换、连锁、下落补充行为不变。
- `GameObject` 更像 session，而不是算法集合。

### Phase 4：引入 ResolveCycle

目标：把玩法解算事实和 UI 表现步骤分开。

任务：

- 新增 `FOrigamiBirdResolveCycle`。
- `BoardResolver` 输出 `ResolveCycles`。
- `MoveResult` / `PropUseResult` 中逐步加入 `ResolveCycles`。
- 旧 `ResolveSteps` 可临时保留，作为兼容层。

验收：

- 编译通过。
- 旧 UI 仍可播放。
- 新结构能够完整描述每一轮匹配、消除、计分、下落、生成。

### Phase 5：引入 PresentationTimeline

目标：让 UI 播放时间线，而不是顺序 Step。

任务：

- 新增 Presentation Types。
- 新增 `OrigamiBirdResolvePresentationCompiler`。
- 新增表现配置 DataAsset 或 DeveloperSettings。
- `BoardWidget` 增加 `PlayPresentationTimeline`。
- 先让 compiler 生成与旧 Step 接近的时序，再逐步支持并行事件。

验收：

- 编译通过。
- 动画播放效果与旧版本接近。
- 能表达 ScorePopup 和 Remove 同时开始。
- `BoardWidget` 不再依赖玩法层硬编码的 ResolveStep 顺序。

### Phase 6：整理道具公共流程

目标：减少 `ApplyProp...` 重复逻辑。

任务：

- 梳理道具效果的公共流程：
  - 校验拥有道具。
  - 校验 Phase。
  - 收集目标。
  - 应用棋盘修改。
  - 可选 ResolveAfterUse。
  - 消耗道具。
  - 生成最终结果。
- 让具体 Effect 更关注“这个道具想改什么”，而不是重复写结算模板。
- 考虑引入通用 `FOrigamiBirdActionResult`，统一交换和道具结果。

验收：

- 编译通过。
- 现有道具行为不变。
- 新增一个简单道具时不需要复制大量模板代码。

### Phase 7：整理 ViewModel 输入状态

目标：让 VM 更像 UI 状态层，而不是混合输入状态机。

任务：

- 抽出普通 Tile 选择状态。
- 抽出道具目标选择状态。
- `MatchScreen` 保持事件连接职责。
- `VM` 保持字段通知和命令转发职责。

验收：

- 编译通过。
- 点击选择、交换、道具目标选择行为不变。
- VM 中复杂分支减少。

## 10. QA 和测试学习计划

自动化测试后置，但重构过程中需要逐步建立测试意识。

后续适合补的测试：

- 初始棋盘没有自然三消。
- 横向三消扫描。
- 纵向三消扫描。
- T 型或 L 型匹配去重。
- 无效交换回滚且不扣步。
- 有效交换扣一步。
- 连锁消除分数倍率。
- 下落补充后没有空洞。
- 固定障碍能阻断下落。
- 道具目标非法时不消耗道具。
- 道具成功时道具数量减少。

学习顺序建议：

1. 先学如何构造固定棋盘。
2. 再学如何验证 `FindAllMatches`。
3. 再学如何验证一次 `Resolve`。
4. 最后再接 Unreal Automation Test。

## 11. 工作原则

每次重构前确认：

- 这一步只解决一个明确问题。
- 是否需要同时改 UI。
- 是否需要同时改道具。
- 是否可以保留兼容层。
- 如果失败，如何回退。

每次重构后记录：

- 改了哪些文件。
- 哪些行为应该保持不变。
- 有哪些人工检查点。
- 有哪些后续清理项。

默认优先级：

```text
职责边界清晰 > 可测试性 > 表现灵活性 > 配置能力 > 代码行数减少
```

## 12. 下一步建议

下一步建议执行 Phase 1：

- 修复乱码注释。
- 给核心函数补充流程注释。
- 不移动函数。
- 不改变类型结构。
- 不改变 UI 播放逻辑。

完成 Phase 1 后，再进入 Phase 2 拆 `FOrigamiBirdBoardState`。
