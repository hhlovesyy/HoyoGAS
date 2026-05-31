#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdBoardState.h"
#include "Core/OrigamiBirdMatchResolver.h"
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

	friend struct FOrigamiBirdPropActionExecutor;
	
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
	bool TrySwapTilesWithResult(FIntPoint From, FIntPoint To, FOrigamiBirdActionResult& OutResult);

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
	bool UsePropWithResult(const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdActionResult& OutResult);
	
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
	FOrigamiBirdBoardState BoardState;
	TArray<FOrigamiBirdPropStack> PropStacks;
	TMap<EOrigamiBirdTileType, FOrigamiBirdTileDefinitionRow> TileDefinitionsByType;

	int32 NextTileId = 1;
	EOrigamiBirdMatchPhase Phase = EOrigamiBirdMatchPhase::None;

	EOrigamiBirdTileType GenerateRandomTileType();
	EOrigamiBirdTileType GenerateRandomTileTypeExcept(EOrigamiBirdTileType ExcludedType);
	void RebuildTileDefinitionMap();

	const FOrigamiBirdTileDefinitionRow* FindTileDefinitionInternal(EOrigamiBirdTileType TileType) const;
	const FOrigamiBirdTileDefinitionRow* FindTileDefinitionForRule(EOrigamiBirdTileType TileType, const TCHAR* RuleName) const;
	bool CanMatchTileType(EOrigamiBirdTileType TileType) const;
	bool CanFallTileType(EOrigamiBirdTileType TileType) const;
	bool CanSwapTileType(EOrigamiBirdTileType TileType) const;
	int32 GetTileScoreValue(EOrigamiBirdTileType TileType) const;
	
	bool AreAdjacent(FIntPoint A, FIntPoint B) const;
	
	FOrigamiBirdCollapseAndRefillResult CollapseAndRefill(); //下落并补充

	//下面的是分数相关的
	int32 Score = 0;
	int32 MovesRemaining = 0;
	int32 UsedMoves = 0;
	int32 MaxCombo = 0;
	int32 RemovedTileCount = 0;

	void CheckGameEnd();
	void BroadcastBoardChanged();
	void BroadcastPropStacksChanged();
	int32 FindPropStackIndex(FName PropId) const;
	
	FOrigamiBirdTile MakeTileSnapshot(FIntPoint Position) const;
	TArray<FOrigamiBirdTile> MakeTileSnapshots(const TArray<FIntPoint>& Positions) const;
	FOrigamiBirdBoardChangeStep MakeSwapStep(FIntPoint From, FIntPoint To) const;
	FOrigamiBirdBoardChangeStep MakeRemoveStep(const TArray<FIntPoint>& Positions) const;
	FOrigamiBirdBoardChangeStep MakeSpawnStep(const TArray<FOrigamiBirdTile>& SpawnedTiles, const TArray<FIntPoint>& SpawnedPositions) const;
	FOrigamiBirdBoardChangeStep MakeFallStep(const TArray<FOrigamiBirdTileTransition>& FallTransitions) const;
	void AppendCollapseSteps(const FOrigamiBirdCollapseAndRefillResult& CollapseResult, FOrigamiBirdActionResult& OutResult) const;
	void ResolveAfterPropUse(FOrigamiBirdActionResult& OutResult);
	FOrigamiBirdMatchResolveResult ResolveCurrentMatches(const FOrigamiBirdResolveSeed* InitialSeed = nullptr);
};
