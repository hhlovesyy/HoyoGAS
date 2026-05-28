#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "OrigamiBirdMatchTypes.h"
#include "OrigamiBirdMatchGameObject.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrigamiBirdBoardChangedEvent, const FOrigamiBirdBoardSnapshot&, Snapshot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrigamiBirdTileSelectedEvent, FIntPoint, BoardPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrigamiBirdScoreChangedEvent, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrigamiBirdMovesChangedEvent, int32, NewMovesRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrigamiBirdPropStacksChangedEvent, const TArray<FOrigamiBirdPropStack>&, PropStacks);

//需要一个 UObject 来持有棋盘。
UCLASS(BlueprintType)
class HOYOGAS_API UOrigamiBirdMatchGameObject : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void Initialize(const FOrigamiBirdMatchStartParams& InStartParams);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void InitializeFromLevelDefinition(const FOrigamiBirdLevelDefinitionRow& LevelDefinition, const TArray<FOrigamiBirdTileDefinitionRow>& TileDefinitions);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void SetTileDefinitions(const TArray<FOrigamiBirdTileDefinitionRow>& InTileDefinitions);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void DumpBoardToLog() const;
	
	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	bool TrySwapTiles(FIntPoint From, FIntPoint To);
	
	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	bool TrySwapTilesWithResult(FIntPoint From, FIntPoint To, FOrigamiBirdMoveResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	bool SelectTile(FIntPoint BoardPosition);
	
	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	FOrigamiBirdBoardSnapshot GetSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	bool SubmitCommand(const FOrigamiBirdMatchCommand& Command);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	bool FindTileDefinition(EOrigamiBirdTileType TileType, FOrigamiBirdTileDefinitionRow& OutDefinition) const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird|Props")
	bool GrantProp(FName PropId, int32 Count, bool bStackable, int32 MaxStackCount);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird|Props")
	bool ConsumeProp(FName PropId, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird|Props")
	int32 GetPropCount(FName PropId) const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird|Props")
	void GetPropStacks(TArray<FOrigamiBirdPropStack>& OutStacks) const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird|Props")
	void ClearProps();

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird|Props")
	bool UsePropWithResult(const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdPropUseResult& OutResult);

	// 道具策略使用的棋盘修改接口：移除一个格子，然后按配置决定是否继续触发三消连锁。
	// 业务入口仍然是 UsePropWithResult，这个函数只给 UOrigamiBirdPropEffect 子类调用。
	bool ApplyPropRemoveSingleTile(FIntPoint TargetPosition, bool bResolveAfterUse, FOrigamiBirdPropUseResult& OutResult);
	bool ApplyPropRandomReplaceTile(FIntPoint TargetPosition, bool bResolveAfterUse, FOrigamiBirdPropUseResult& OutResult);
	bool ApplyPropSwapColumns(int32 FirstColumn, int32 SecondColumn, bool bResolveAfterUse, FOrigamiBirdPropUseResult& OutResult);
	bool ApplyPropCopyColumnToNeighbor(int32 SourceColumn, bool bResolveAfterUse, FOrigamiBirdPropUseResult& OutResult);
	bool ApplyPropShuffleBoard(bool bResolveAfterUse, FOrigamiBirdPropUseResult& OutResult);
	bool ApplyPropExplode3x3(FIntPoint CenterPosition, bool bResolveAfterUse, FOrigamiBirdPropUseResult& OutResult);
	
	//相关的事件
	UPROPERTY(BlueprintAssignable, Category = "OrigamiBird")
	FOrigamiBirdBoardChangedEvent OnBoardChanged;

	UPROPERTY(BlueprintAssignable, Category = "OrigamiBird")
	FOrigamiBirdTileSelectedEvent OnTileSelected;

	UPROPERTY(BlueprintAssignable, Category = "OrigamiBird")
	FOrigamiBirdScoreChangedEvent OnScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "OrigamiBird")
	FOrigamiBirdMovesChangedEvent OnMovesChanged;

	UPROPERTY(BlueprintAssignable, Category = "OrigamiBird")
	FOrigamiBirdPropStacksChangedEvent OnPropStacksChanged;
	
private:
	FOrigamiBirdMatchStartParams StartParams;
	FRandomStream RandomStream;
	TArray<FOrigamiBirdTile> BoardTiles;
	TArray<FOrigamiBirdPropStack> PropStacks;
	TMap<EOrigamiBirdTileType, FOrigamiBirdTileDefinitionRow> TileDefinitionsByType;

	int32 NextTileId = 1;
	EOrigamiBirdMatchPhase Phase = EOrigamiBirdMatchPhase::None;

	int32 ToIndex(FIntPoint Position) const;
	bool IsInsideBoard(FIntPoint Position) const;
	bool IsInsideColumn(int32 Column) const;
	EOrigamiBirdTileType GenerateRandomTileType();
	EOrigamiBirdTileType GenerateRandomTileTypeExcept(EOrigamiBirdTileType ExcludedType);
	void GenerateInitialBoard();
	void RebuildTileDefinitionMap();

	const FOrigamiBirdTile* GetTile(FIntPoint Position) const;
	FOrigamiBirdTile* GetTile(FIntPoint Position);
	const FOrigamiBirdTileDefinitionRow* FindTileDefinitionInternal(EOrigamiBirdTileType TileType) const;
	bool CanMatchTileType(EOrigamiBirdTileType TileType) const;
	bool CanFallTileType(EOrigamiBirdTileType TileType) const;
	bool CanSwapTileType(EOrigamiBirdTileType TileType) const;
	int32 GetTileScoreValue(EOrigamiBirdTileType TileType) const;
	
	bool WouldCreateMatchAt(FIntPoint Position) const; //初始化棋盘时，不能生成已有三消。
	TArray<FIntPoint> FindAllMatches() const; //找到所有三消的位置
	
	bool AreAdjacent(FIntPoint A, FIntPoint B) const;
	void SwapTileData(FIntPoint A, FIntPoint B);
	
	void RemoveTiles(const TArray<FIntPoint>& Positions);
	
	void CollapseAndRefill(TArray<FOrigamiBirdResolveStep>* OutSteps = nullptr); //下落并补充

	//下面的是分数相关的
	int32 Score = 0;
	int32 MovesRemaining = 0;
	int32 UsedMoves = 0;
	int32 MaxCombo = 0;
	int32 RemovedTileCount = 0;

	void CheckGameEnd();
	void BroadcastBoardChanged();
	void BroadcastPropStacksChanged();
	void ClearSelection();
	int32 FindPropStackIndex(FName PropId) const;
	
	//记录step的辅助函数
	FOrigamiBirdTile MakeTileSnapshot(FIntPoint Position) const;
	TArray<FOrigamiBirdTile> MakeTileSnapshots(const TArray<FIntPoint>& Positions) const;
	FOrigamiBirdResolveStep MakeSwapStep(FIntPoint From, FIntPoint To) const;
	FOrigamiBirdResolveStep MakeMatchStep(const TArray<FIntPoint>& MatchPositions, int32 ComboIndex) const;
	FOrigamiBirdResolveStep MakeRemoveStep(const TArray<FIntPoint>& MatchPositions, int32 ComboIndex) const;
	FOrigamiBirdResolveStep MakeScoreStep(int32 ScoreDelta, int32 ComboIndex, int32 InRemovedTileCount) const;
	void ResolveCurrentMatchesIntoSteps(TArray<FOrigamiBirdResolveStep>& OutSteps, int32& InOutTotalScoreDelta, int32& InOutTotalRemovedTileCount, int32& OutMaxComboIndex);
};
