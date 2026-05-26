#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Core/OrigamiBirdMatchTypes.h"
#include "OrigamiBirdMatchSubsystem.generated.h"

class UDataTable;
class UOrigamiBirdMatchGameObject;

// 折纸小鸟对对碰玩法入口。
// 它负责读取策划表、创建当前一局游戏，并把当前 Match 暴露给 UI/ViewModel。
// 真正的棋盘规则仍然放在 UOrigamiBirdMatchGameObject，不要把交换/消除/下落规则塞进 Subsystem。
UCLASS()
class HOYOGAS_API UOrigamiBirdMatchSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	// 使用 Project Settings 里的 DefaultDevelopmentLevelId 启动一局，适合开发期快捷键或 Debug UI。
	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	UOrigamiBirdMatchGameObject* StartDefaultDevelopmentMatch();

	// 按关卡表 RowName 启动一局。比如 Level_001。
	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	UOrigamiBirdMatchGameObject* StartMatchByLevelId(FName LevelId);

	// 结束当前局。当前只清掉内存对象；未来接存档/结算时可以在这里扩展。
	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void EndActiveMatch();

	// 当前正在运行的一局。UI/VM 可以绑定它的 OnBoardChanged。
	UFUNCTION(BlueprintPure, Category = "OrigamiBird")
	UOrigamiBirdMatchGameObject* GetActiveMatch() const;

	// 从关卡表读取一行定义。
	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	bool FindLevelDefinition(FName LevelId, FOrigamiBirdLevelDefinitionRow& OutDefinition) const;

	// 从方块表按 TileType 查定义，UI 显示图标/颜色时会用到。
	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	bool FindTileDefinition(EOrigamiBirdTileType TileType, FOrigamiBirdTileDefinitionRow& OutDefinition) const;

	// 读取全部方块定义。启动一局时会传给 GameObject。
	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void GetAllTileDefinitions(TArray<FOrigamiBirdTileDefinitionRow>& OutDefinitions) const;

	// 读取全部关卡 ID，后续关卡选择 UI 可以用。
	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void GetAllLevelIds(TArray<FName>& OutLevelIds) const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	UDataTable* LoadTileDefinitionTable() const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	UDataTable* LoadLevelDefinitionTable() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UOrigamiBirdMatchGameObject> ActiveMatch;
};
