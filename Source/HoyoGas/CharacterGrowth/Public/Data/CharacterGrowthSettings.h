#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CharacterGrowthSettings.generated.h"

class UDataTable;

// 角色养成系统配置：集中指定遗器、光锥、行迹、星魂等策划表资产。
// 当前先接入遗器三张核心表，后续武器/光锥也会放在这里。
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Hoyo Character Growth"))
class HOYOGAS_API UCharacterGrowthSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;

	// 遗器套装表：定义套装名称、二件套、四件套效果。
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Relic")
	TSoftObjectPtr<UDataTable> RelicSetDefinitionTable;

	// 遗器部件表：定义每个遗器模板属于哪个套装、哪个槽位、叫什么、图标是什么。
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Relic")
	TSoftObjectPtr<UDataTable> RelicDefinitionTable;

	// 遗器词条表：定义暴击率、攻击力百分比、速度等词条如何显示和映射到 GAS 属性。
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Relic")
	TSoftObjectPtr<UDataTable> RelicAffixDefinitionTable;

	// 是否允许遗器开发测试入口。关闭后 CharacterPanel 上的测试按钮不会生成遗器；Shipping 包也会强制禁用。
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Relic|Development")
	bool bEnableRelicDevelopmentActions = true;

	// 打开角色面板时，是否自动给当前角色补齐开发测试遗器。
	// 仅用于验证遗器表、装备组件、CharacterPanel UI 的链路；正式项目接存档后应关闭。
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Relic|Development")
	bool bAutoGrantRelicDevelopmentLoadoutOnCharacterPanelOpen = true;
};
