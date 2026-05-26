# Origami Bird DataTable JSON Examples

## TileDefinition 表

DataTable Row Struct 选择 `FOrigamiBirdTileDefinitionRow`。

```json
[
  {
    "Name": "RedFruit",
    "TileType": "RedFruit",
    "DisplayName": "红色果实",
    "Icon": "/Game/OrigamiBird/Icons/T_RedFruit.T_RedFruit",
    "DebugColor": "(R=1.000000,G=0.100000,B=0.100000,A=1.000000)",
    "bCanMatch": true,
    "bCanFall": true,
    "bCanSwap": true,
    "ScoreValue": 10
  },
  {
    "Name": "BlueFruit",
    "TileType": "BlueFruit",
    "DisplayName": "蓝色果实",
    "Icon": "/Game/OrigamiBird/Icons/T_BlueFruit.T_BlueFruit",
    "DebugColor": "(R=0.100000,G=0.350000,B=1.000000,A=1.000000)",
    "bCanMatch": true,
    "bCanFall": true,
    "bCanSwap": true,
    "ScoreValue": 10
  },
  {
    "Name": "GreenFruit",
    "TileType": "GreenFruit",
    "DisplayName": "绿色果实",
    "Icon": "/Game/OrigamiBird/Icons/T_GreenFruit.T_GreenFruit",
    "DebugColor": "(R=0.100000,G=0.900000,B=0.250000,A=1.000000)",
    "bCanMatch": true,
    "bCanFall": true,
    "bCanSwap": true,
    "ScoreValue": 10
  },
  {
    "Name": "YellowFruit",
    "TileType": "YellowFruit",
    "DisplayName": "黄色果实",
    "Icon": "/Game/OrigamiBird/Icons/T_YellowFruit.T_YellowFruit",
    "DebugColor": "(R=1.000000,G=0.850000,B=0.100000,A=1.000000)",
    "bCanMatch": true,
    "bCanFall": true,
    "bCanSwap": true,
    "ScoreValue": 10
  },
  {
    "Name": "PurpleFruit",
    "TileType": "PurpleFruit",
    "DisplayName": "紫色果实",
    "Icon": "/Game/OrigamiBird/Icons/T_PurpleFruit.T_PurpleFruit",
    "DebugColor": "(R=0.650000,G=0.250000,B=1.000000,A=1.000000)",
    "bCanMatch": true,
    "bCanFall": true,
    "bCanSwap": true,
    "ScoreValue": 10
  }
]
```

## LevelDefinition 表

DataTable Row Struct 选择 `FOrigamiBirdLevelDefinitionRow`。

```json
[
  {
    "Name": "Level_001",
    "DisplayName": "测试关卡 001",
    "BoardWidth": 7,
    "BoardHeight": 7,
    "MoveLimit": 25,
    "TargetScore": 3000,
    "RandomSeed": 12345,
    "AvailableTileTypes": [
      "RedFruit",
      "BlueFruit",
      "GreenFruit",
      "YellowFruit",
      "PurpleFruit"
    ]
  }
]
```

## 字段含义

- `TileType`：棋盘里真正保存的类型。
- `Icon`：UI 图标资源路径。
- `bCanMatch`：是否能组成三消。
- `bCanFall`：是否会受重力下落。
- `bCanSwap`：是否允许玩家交换。
- `ScoreValue`：单个方块消除基础分。
- `AvailableTileTypes`：本关自然生成池，通常只放普通可消除方块。
