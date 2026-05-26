#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "OrigamiBirdSettings.generated.h"

class UDataTable;

// 折纸小鸟对对碰的项目级配置。
// 这里集中指定方块表、关卡表，后续活动入口、Debug UI、关卡系统都从这里找默认策划表。
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Origami Bird Match"))
class HOYOGAS_API UOrigamiBirdSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;

	// 方块定义表：每行是一个 TileDefinition，比如红色水果、蓝色水果、冰块、石头。
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> TileDefinitionTable;

	// 关卡定义表：每行是一个 LevelDefinition，比如 Level_001、Level_002。
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> LevelDefinitionTable;

	// 开发测试默认关卡。Debug UI 或快捷键启动玩法时可以先读这行。
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Development")
	FName DefaultDevelopmentLevelId = TEXT("Level_001");
};
