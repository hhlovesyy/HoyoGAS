#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "OrigamiBirdMatchTypes.h"
#include "OrigamiBirdMatchGameObject.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrigamiBirdBoardChangedEvent, const FOrigamiBirdBoardSnapshot&, Snapshot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrigamiBirdTileSelectedEvent, FIntPoint, BoardPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrigamiBirdScoreChangedEvent, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrigamiBirdMovesChangedEvent, int32, NewMovesRemaining);

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
	bool SelectTile(FIntPoint BoardPosition);
	
	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	FOrigamiBirdBoardSnapshot GetSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	bool SubmitCommand(const FOrigamiBirdMatchCommand& Command);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	bool FindTileDefinition(EOrigamiBirdTileType TileType, FOrigamiBirdTileDefinitionRow& OutDefinition) const;
	
	//相关的事件
	UPROPERTY(BlueprintAssignable, Category = "OrigamiBird")
	FOrigamiBirdBoardChangedEvent OnBoardChanged;

	UPROPERTY(BlueprintAssignable, Category = "OrigamiBird")
	FOrigamiBirdTileSelectedEvent OnTileSelected;

	UPROPERTY(BlueprintAssignable, Category = "OrigamiBird")
	FOrigamiBirdScoreChangedEvent OnScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "OrigamiBird")
	FOrigamiBirdMovesChangedEvent OnMovesChanged;
	
private:
	FOrigamiBirdMatchStartParams StartParams;
	FRandomStream RandomStream;
	TArray<FOrigamiBirdTile> BoardTiles;
	TMap<EOrigamiBirdTileType, FOrigamiBirdTileDefinitionRow> TileDefinitionsByType;

	int32 NextTileId = 1;
	EOrigamiBirdMatchPhase Phase = EOrigamiBirdMatchPhase::None;

	int32 ToIndex(FIntPoint Position) const;
	bool IsInsideBoard(FIntPoint Position) const;
	EOrigamiBirdTileType GenerateRandomTileType();
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
	
	void CollapseAndRefill(); //下落并补充
	bool ResolveBoardAfterValidSwap(); //有可能会连续消除，这个是连续消除的入口

	//下面的是分数相关的
	int32 Score = 0;
	int32 MovesRemaining = 0;
	int32 UsedMoves = 0;
	int32 MaxCombo = 0;
	int32 RemovedTileCount = 0;

	void CheckGameEnd();
	void BroadcastBoardChanged();
	void ClearSelection();
};
