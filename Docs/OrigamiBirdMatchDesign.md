# 折纸小鸟对对碰学习版 玩法策划案 v0.1

## 1. 项目目标

本玩法是一个接入现有活动界面的三消小游戏学习项目。目标不是完整复刻商业活动，而是用一个小体量、可扩展的三消活动练习以下能力：

- 棋盘 gameplay 规则：交换、匹配、消除、掉落、补充、连锁。
- UI 系统：活动入口、棋盘展示、Tile 点击、选中态、分数、道具栏、结算页。
- 数据表驱动：Tile 类型、关卡目标、道具、小鸟技能。
- 事件驱动：棋盘事件驱动 VM 刷新，结算事件写入 Progression。
- 后续 GAS 接入：把道具、Buff、被动技能做成可配置效果。

第一阶段只做单人本地玩法，不做联网、不做 PVP、不做复杂剧情包装。

## 2. 核心体验

玩家从活动界面进入小游戏，看到一个水果棋盘。玩家交换相邻 Tile，形成三个或以上同色连线后消除。消除会获得分数、触发掉落和补充。连续消除会形成连锁，连锁越高分数越高。达到关卡目标后进入结算页。

核心循环：

```text
选择 Tile
-> 交换相邻 Tile
-> 检测匹配
-> 消除并计分
-> 掉落与补充
-> 检测连锁
-> 更新目标进度
-> 达成目标或继续操作
```

## 3. MVP 范围

第一版只做这些：

- 6 x 6 棋盘。
- 5 种普通 Tile。
- 相邻交换。
- 横向或纵向 3 连及以上消除。
- 消除后上方 Tile 下落，顶部随机补充。
- 连锁计数。
- 分数。
- 剩余步数。
- 一个关卡目标：指定分数或指定 Tile 消除数量。
- 一个活动入口按钮。
- 一个局内页面。
- 一个结算页面。

第一版暂不做：

- PVP。
- 复杂 AI。
- 多局排行榜。
- 复杂剧情演出。
- GAS。
- 存档型养成。
- 商业活动完整资源包装。

## 4. 棋盘规则

棋盘使用二维坐标：

```text
X: 0 到 Width - 1
Y: 0 到 Height - 1
```

建议约定：

- 左上角是 `(0, 0)`。
- X 向右增加。
- Y 向下增加。
- UI 展示顺序由 VM 转换，不让 UI 自己推算 gameplay 坐标。

每个格子保存一个 Tile：

```text
TileId
TileType
BoardPosition
bIsMatched
bIsFalling
bIsSelected
```

## 5. Tile 类型

MVP 使用 5 种普通 Tile：

```text
RedFruit
BlueFruit
YellowFruit
GreenFruit
PurpleFruit
```

后续扩展特殊 Tile：

```text
RowBomb       横向炸弹
ColumnBomb    纵向炸弹
ColorBomb     消除同色
BirdPower     小鸟技能充能 Tile
Blocker       障碍 Tile
```

## 6. 匹配规则

一次有效匹配满足：

- 横向连续 3 个或以上同类型普通 Tile。
- 纵向连续 3 个或以上同类型普通 Tile。
- 特殊 Tile 是否参与匹配由后续规则决定，MVP 暂不处理。

检测流程：

```text
扫描每一行，找连续相同 TileType 的片段。
扫描每一列，找连续相同 TileType 的片段。
长度 >= 3 的片段加入 MatchedTiles。
去重。
返回 MatchedTiles。
```

交换后如果没有产生匹配，则交换回去。

## 7. 消除、掉落与补充

消除流程：

```text
标记 MatchedTiles
播放消除表现
移除 MatchedTiles
每一列从下往上压缩
顶部生成新 Tile
播放掉落表现
再次检测匹配
如果有匹配，进入连锁
```

MVP 可以先不做复杂动画，先保证数据正确。动画后续由 UI 层根据事件补。

## 8. 分数规则

基础分数：

```text
每消除 1 个 Tile = 10 分
```

连锁倍率：

```text
第 1 次消除：x1.0
第 2 次连锁：x1.2
第 3 次连锁：x1.5
第 4 次及以上：x2.0
```

计分公式：

```text
Score += RemovedTileCount * 10 * ComboMultiplier
```

MVP 中分数只用于通关目标，不接战斗数值。

## 9. 步数与失败条件

每次有效交换消耗 1 步。

无效交换不消耗步数。

失败条件：

```text
剩余步数为 0
且未达成关卡目标
```

成功条件：

```text
达到目标分数
或达到指定 Tile 消除数量
```

## 10. 道具规则

MVP 可以先留 UI 位置，不实现道具。第二阶段实现 3 个道具：

### 横排清除

选择一个格子，清除该格所在整行。

### 竖列清除

选择一个格子，清除该格所在整列。

### 随机刷新

重新生成全棋盘普通 Tile，并保证初始棋盘没有自动匹配。

道具第一版可以直接由 GameObject 处理。后续如果要练 GAS，可以把道具效果抽成 GameplayEffect-like 的配置。

## 11. 小鸟技能

小鸟技能不是 MVP 必须项。第二阶段加入。

示例技能：

```text
红色小鸟：消除红色 Tile 时额外加分。
蓝色小鸟：每 3 次连锁生成一个横排清除道具。
黄色小鸟：开局额外获得 3 步。
绿色小鸟：每次消除 5 个以上 Tile 时回复技能能量。
```

技能可以先用普通 C++ 策略类或数据表处理。第三阶段再考虑 GAS。

## 12. 活动 UI 流程

页面流程：

```text
活动入口页
-> 小游戏规则/开始页
-> 局内三消页
-> 结算页
-> 返回活动入口页
```

局内页面元素：

- 棋盘区域。
- 当前分数。
- 目标分数或目标进度。
- 剩余步数。
- 当前连锁提示。
- 道具栏预留。
- 暂停/退出按钮。

结算页面元素：

- 成功/失败。
- 本局分数。
- 最大连锁。
- 消除总数。
- 奖励领取状态。

## 13. 技术边界

第一版不需要 Store。

推荐结构：

```text
UOrigamiBirdMatchGameObject
    临时 UObject，负责棋盘和规则。

UVM_OrigamiBirdMatchScreen
    持有 GameObject，订阅事件，输出 UI 字段。

UVM_OrigamiBirdTileEntry
    每个 Tile 的 UI 数据。

WBP_OrigamiBirdMatchScreen / JSON
    展示棋盘、分数、按钮和结算。

Activity Screen
    作为入口，打开小游戏页面。
```

不建议第一版做常驻 Subsystem。只有当多个页面、多局状态、活动奖励、Progression 都要共享时，再引入活动 Subsystem。

## 14. 数据表设计

### Tile 定义表

```text
TileType
DisplayName
Icon
Color
bCanMatch
bCanFall
bCanBeRemoved
```

### 关卡定义表

```text
LevelId
BoardWidth
BoardHeight
MoveLimit
TargetScore
TargetTileType
TargetTileCount
AvailableTileTypes
InitialPowerUps
```

### 道具定义表

```text
PowerUpId
DisplayName
Icon
Description
TargetType
EffectType
Cooldown
```

### 小鸟技能定义表

```text
BirdId
DisplayName
Icon
PassiveDescription
ActiveSkillDescription
TriggerType
EffectType
Value
```

## 15. 事件设计

GameObject 对外广播事件：

```text
OnBoardInitialized
OnTileSelected
OnSwapStarted
OnSwapResolved
OnTilesMatched
OnTilesRemoved
OnTilesDropped
OnComboChanged
OnScoreChanged
OnMovesChanged
OnObjectiveChanged
OnGameEnded
```

VM 订阅这些事件并刷新绑定字段。

UI 不直接修改棋盘数组，只调用：

```text
SelectTile(TileId)
TrySwapSelectedWith(TileId)
UsePowerUp(PowerUpId, TargetTileId)
RestartLevel()
ExitGame()
```

## 16. GAS 接入边界

MVP 不接 GAS。

适合接 GAS 的部分：

- 道具效果。
- 小鸟被动技能。
- 分数倍率 Buff。
- 临时状态，例如 `NextSwapFree`。
- 连锁触发效果。
- 道具冷却和充能。

不适合接 GAS 的部分：

- 棋盘二维数组。
- 匹配扫描算法。
- Tile 掉落。
- UI 动画。

推荐第三阶段接入：

```text
棋盘事件
-> Match GameObject 生成玩法事件
-> Ability / Effect 响应事件
-> 修改局内临时状态
-> VM 刷新 UI
```

## 17. Progression 接入

第一版只在结算时写一次进度：

```text
Activity.OrigamiBird.CompletedGames += 1
Activity.OrigamiBird.BestScore = Max(BestScore, CurrentScore)
Activity.OrigamiBird.TotalRemovedTiles += RemovedTiles
```

可扩展成成就：

```text
单局达到 5000 分
单局触发 5 次连锁
累计消除 1000 个 Tile
使用 20 次道具
```

## 18. 开发阶段拆分

### 阶段 1：纯规则原型

- GameObject。
- 棋盘生成。
- 交换。
- 匹配。
- 消除。
- 掉落。
- 补充。
- 控制台或简单 UI 打印结果。

### 阶段 2：接 UI

- TileEntry VM。
- 棋盘显示。
- 点击选择。
- 分数和步数。
- 结算页。

### 阶段 3：活动入口

- 活动页按钮。
- 打开小游戏页。
- 退出返回活动页。
- 结算写 Progression。

### 阶段 4：道具与小鸟技能

- 3 个基础道具。
- 2 个小鸟被动。
- 数据表配置。

### 阶段 5：GAS 练习版

- 把道具和小鸟技能的一部分效果接入 GAS。
- 棋盘核心逻辑仍然保持普通 C++。

## 19. 验收标准

MVP 完成标准：

- 可以从活动界面进入三消页。
- 棋盘能正确生成。
- 可以点击相邻 Tile 交换。
- 有效交换能消除。
- 无效交换会复原。
- 消除后能掉落和补充。
- 连锁能继续结算。
- 分数、步数、目标显示正确。
- 成功或失败后进入结算页。
- 退出后不保留局内临时对象。

## 20. 给外部模型继续扩写的提示词

如果需要让其他模型扩写，不要让它重新发散需求。使用下面提示词：

```text
你是游戏活动玩法策划和 UE5 客户端技术策划。请基于下面这份《折纸小鸟对对碰学习版 玩法策划案 v0.1》继续扩写，但必须遵守这些约束：

1. 这是学习项目，不是完整商业复刻。
2. 第一版只做 6x6 单人三消 MVP。
3. 不要引入联网、PVP、复杂剧情、排行榜。
4. 技术结构保持：临时 GameObject + ViewModel + JSON/WBP UI。
5. 第一版不使用 GAS；后续只把道具、Buff、小鸟技能接入 GAS。
6. 不要改变核心玩法边界，只能补充细节、风险点、数据表示例和验收标准。
7. 输出 Markdown。

请扩写以下部分：
- Tile 数据表示例。
- 关卡数据表示例。
- 道具数据表示例。
- UI 页面草图说明。
- 开发任务拆分。
- 测试用例。
```
