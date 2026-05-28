#include "Core/OrigamiBirdMatchGameObject.h"

#include "Core/OrigamiBirdPropEffect.h"

//在当前的 C++ 文件（.cpp）中，定义一个专属的自定义日志类别（Log Category）
DEFINE_LOG_CATEGORY_STATIC(LogOrigamiBirdMatch, Log, All);

void UOrigamiBirdMatchGameObject::Initialize(const FOrigamiBirdMatchStartParams& InStartParams)
{
	StartParams = InStartParams;

	if (StartParams.AvailableTileTypes.IsEmpty())
	{
		StartParams.AvailableTileTypes =
		{
			EOrigamiBirdTileType::RedFruit,
			EOrigamiBirdTileType::BlueFruit,
			EOrigamiBirdTileType::YellowFruit,
			EOrigamiBirdTileType::GreenFruit,
			EOrigamiBirdTileType::PurpleFruit
		};
	}

	RebuildTileDefinitionMap();
	StartParams.BoardWidth = FMath::Clamp(StartParams.BoardWidth, 3, 12);
	StartParams.BoardHeight = FMath::Clamp(StartParams.BoardHeight, 3, 12);
	RandomStream.Initialize(StartParams.RandomSeed);

	BoardTiles.Reset();
	PropStacks.Reset();
	NextTileId = 1;

	Score = 0;
	MovesRemaining = FMath::Max(1, StartParams.MoveLimit);
	UsedMoves = 0;
	MaxCombo = 0;
	RemovedTileCount = 0;
	Phase = EOrigamiBirdMatchPhase::WaitingInput;

	GenerateInitialBoard();
	BroadcastBoardChanged();
}

void UOrigamiBirdMatchGameObject::InitializeFromLevelDefinition(const FOrigamiBirdLevelDefinitionRow& LevelDefinition, const TArray<FOrigamiBirdTileDefinitionRow>& TileDefinitions)
{
	FOrigamiBirdMatchStartParams Params = LevelDefinition.ToStartParams();
	Params.TileDefinitions = TileDefinitions;
	Initialize(Params);
}

void UOrigamiBirdMatchGameObject::SetTileDefinitions(const TArray<FOrigamiBirdTileDefinitionRow>& InTileDefinitions)
{
	StartParams.TileDefinitions = InTileDefinitions;
	RebuildTileDefinitionMap();
}

void UOrigamiBirdMatchGameObject::RebuildTileDefinitionMap()
{
	TileDefinitionsByType.Reset();

	for (const FOrigamiBirdTileDefinitionRow& Definition : StartParams.TileDefinitions)
	{
		if (Definition.TileType != EOrigamiBirdTileType::None)
		{
			TileDefinitionsByType.Add(Definition.TileType, Definition);
		}
	}
}

int32 UOrigamiBirdMatchGameObject::ToIndex(FIntPoint Position) const
{
	//X = 第几列，从左到右增加
	//Y = 第几行，从上到下增加
	//原点在左上角
	return Position.Y * StartParams.BoardWidth + Position.X;
}

bool UOrigamiBirdMatchGameObject::IsInsideBoard(FIntPoint Position) const
{
	return Position.X >= 0
		&& Position.Y >= 0
		&& Position.X < StartParams.BoardWidth
		&& Position.Y < StartParams.BoardHeight;
}

bool UOrigamiBirdMatchGameObject::IsInsideColumn(int32 Column) const
{
	return Column >= 0 && Column < StartParams.BoardWidth;
}

const FOrigamiBirdTile* UOrigamiBirdMatchGameObject::GetTile(FIntPoint Position) const
{
	return IsInsideBoard(Position) ? &BoardTiles[ToIndex(Position)] : nullptr;
}

FOrigamiBirdTile* UOrigamiBirdMatchGameObject::GetTile(FIntPoint Position)
{
	return IsInsideBoard(Position) ? &BoardTiles[ToIndex(Position)] : nullptr;
}

const FOrigamiBirdTileDefinitionRow* UOrigamiBirdMatchGameObject::FindTileDefinitionInternal(EOrigamiBirdTileType TileType) const
{
	return TileDefinitionsByType.Find(TileType);
}

bool UOrigamiBirdMatchGameObject::FindTileDefinition(EOrigamiBirdTileType TileType, FOrigamiBirdTileDefinitionRow& OutDefinition) const
{
	if (const FOrigamiBirdTileDefinitionRow* Definition = FindTileDefinitionInternal(TileType))
	{
		OutDefinition = *Definition;
		return true;
	}

	return false;
}

bool UOrigamiBirdMatchGameObject::GrantProp(FName PropId, int32 Count, bool bStackable, int32 MaxStackCount)
{
	if (PropId.IsNone() || Count <= 0)
	{
		return false;
	}

	const int32 EffectiveMaxStackCount = bStackable ? FMath::Max(1, MaxStackCount) : 1;
	const int32 ExistingIndex = FindPropStackIndex(PropId);

	if (ExistingIndex != INDEX_NONE)
	{
		FOrigamiBirdPropStack& Stack = PropStacks[ExistingIndex];
		const int32 OldCount = Stack.Count;
		Stack.Count = FMath::Clamp(Stack.Count + Count, 0, EffectiveMaxStackCount);

		if (Stack.Count != OldCount)
		{
			BroadcastPropStacksChanged();
			return true;
		}

		return false;
	}

	FOrigamiBirdPropStack NewStack;
	NewStack.PropId = PropId;
	NewStack.Count = FMath::Clamp(Count, 0, EffectiveMaxStackCount);
	if (NewStack.Count <= 0)
	{
		return false;
	}

	PropStacks.Add(NewStack);
	BroadcastPropStacksChanged();
	return true;
}

bool UOrigamiBirdMatchGameObject::ConsumeProp(FName PropId, int32 Count)
{
	if (PropId.IsNone() || Count <= 0)
	{
		return false;
	}

	const int32 ExistingIndex = FindPropStackIndex(PropId);
	if (ExistingIndex == INDEX_NONE)
	{
		return false;
	}

	FOrigamiBirdPropStack& Stack = PropStacks[ExistingIndex];
	if (Stack.Count < Count)
	{
		return false;
	}

	Stack.Count -= Count;
	if (Stack.Count <= 0)
	{
		PropStacks.RemoveAt(ExistingIndex);
	}

	BroadcastPropStacksChanged();
	return true;
}

int32 UOrigamiBirdMatchGameObject::GetPropCount(FName PropId) const
{
	const int32 ExistingIndex = FindPropStackIndex(PropId);
	return ExistingIndex != INDEX_NONE ? PropStacks[ExistingIndex].Count : 0;
}

void UOrigamiBirdMatchGameObject::GetPropStacks(TArray<FOrigamiBirdPropStack>& OutStacks) const
{
	OutStacks = PropStacks;
}

void UOrigamiBirdMatchGameObject::ClearProps()
{
	if (PropStacks.IsEmpty())
	{
		return;
	}

	PropStacks.Reset();
	BroadcastPropStacksChanged();
}

bool UOrigamiBirdMatchGameObject::UsePropWithResult(const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdPropUseResult& OutResult)
{
	OutResult = FOrigamiBirdPropUseResult();
	OutResult.PropId = Request.PropId;
	OutResult.InitialSnapshot = GetSnapshot();

	if (Request.PropId.IsNone())
	{
		OutResult.FailureReasonId = TEXT("InvalidPropId");
		OutResult.FinalSnapshot = GetSnapshot();
		return false;
	}

	if (Phase != EOrigamiBirdMatchPhase::WaitingInput)
	{
		OutResult.FailureReasonId = TEXT("InvalidPhase");
		OutResult.FinalSnapshot = GetSnapshot();
		return false;
	}

	if (GetPropCount(Request.PropId) <= 0)
	{
		OutResult.FailureReasonId = TEXT("PropNotOwned");
		OutResult.FinalSnapshot = GetSnapshot();
		return false;
	}

	if (!Definition.EffectClass)
	{
		OutResult.FailureReasonId = TEXT("MissingEffectClass");
		OutResult.FinalSnapshot = GetSnapshot();
		return false;
	}

	const UOrigamiBirdPropEffect* Effect = Definition.EffectClass->GetDefaultObject<UOrigamiBirdPropEffect>();
	if (!Effect)
	{
		OutResult.FailureReasonId = TEXT("InvalidEffectClass");
		OutResult.FinalSnapshot = GetSnapshot();
		return false;
	}

	Phase = EOrigamiBirdMatchPhase::Resolving;
	ClearSelection();

	const bool bExecuted = Effect->Execute(this, Definition, Request, OutResult);
	if (!bExecuted || !OutResult.bAccepted)
	{
		Phase = EOrigamiBirdMatchPhase::WaitingInput;
		OutResult.bAccepted = false;
		OutResult.FinalSnapshot = GetSnapshot();
		BroadcastBoardChanged();
		return false;
	}

	ConsumeProp(Request.PropId, 1);

	Phase = EOrigamiBirdMatchPhase::WaitingInput;
	CheckGameEnd();
	OutResult.FinalSnapshot = GetSnapshot();

	FOrigamiBirdResolveStep FinalStep;
	FinalStep.StepType = EOrigamiBirdResolveStepType::FinalSnapshot;
	FinalStep.SnapshotAfterStep = OutResult.FinalSnapshot;
	OutResult.ResolveSteps.Add(FinalStep);

	BroadcastBoardChanged();
	return true;
}

bool UOrigamiBirdMatchGameObject::ApplyPropRemoveSingleTile(FIntPoint TargetPosition, bool bResolveAfterUse, FOrigamiBirdPropUseResult& OutResult)
{
	if (!IsInsideBoard(TargetPosition))
	{
		OutResult.FailureReasonId = TEXT("OutOfBoard");
		return false;
	}

	const FOrigamiBirdTile* TargetTile = GetTile(TargetPosition);
	if (!TargetTile || TargetTile->TileType == EOrigamiBirdTileType::None)
	{
		OutResult.FailureReasonId = TEXT("EmptyTile");
		return false;
	}

	TArray<FIntPoint> RemovedPositions;
	RemovedPositions.Add(TargetPosition);

	FOrigamiBirdResolveStep RemoveStep = MakeRemoveStep(RemovedPositions, 0);
	RemoveStep.SnapshotAfterStep = GetSnapshot();
	OutResult.ResolveSteps.Add(RemoveStep);

	RemoveTiles(RemovedPositions);
	RemovedTileCount += RemovedPositions.Num();
	OutResult.RemovedTileCount += RemovedPositions.Num();

	CollapseAndRefill(&OutResult.ResolveSteps);

	if (bResolveAfterUse)
	{
		int32 ResolveScoreDelta = 0;
		int32 ResolveRemovedTileCount = 0;
		int32 ResolveMaxComboIndex = 0;
		ResolveCurrentMatchesIntoSteps(OutResult.ResolveSteps, ResolveScoreDelta, ResolveRemovedTileCount, ResolveMaxComboIndex);
		OutResult.TotalScoreDelta += ResolveScoreDelta;
		OutResult.RemovedTileCount += ResolveRemovedTileCount;
	}

	OutResult.bAccepted = true;
	return true;
}

bool UOrigamiBirdMatchGameObject::ApplyPropRandomReplaceTile(FIntPoint TargetPosition, bool bResolveAfterUse, FOrigamiBirdPropUseResult& OutResult)
{
	if (!IsInsideBoard(TargetPosition))
	{
		OutResult.FailureReasonId = TEXT("OutOfBoard");
		return false;
	}

	FOrigamiBirdTile* TargetTile = GetTile(TargetPosition);
	if (!TargetTile || TargetTile->TileType == EOrigamiBirdTileType::None)
	{
		OutResult.FailureReasonId = TEXT("EmptyTile");
		return false;
	}

	TArray<FIntPoint> Positions;
	Positions.Add(TargetPosition);

	FOrigamiBirdResolveStep RemoveStep = MakeRemoveStep(Positions, 0);
	RemoveStep.SnapshotAfterStep = GetSnapshot();
	OutResult.ResolveSteps.Add(RemoveStep);

	TargetTile->TileType = GenerateRandomTileTypeExcept(TargetTile->TileType);
	TargetTile->TileId = NextTileId++;
	TargetTile->BoardPosition = TargetPosition;
	TargetTile->bIsSelected = false;

	FOrigamiBirdResolveStep SpawnStep;
	SpawnStep.StepType = EOrigamiBirdResolveStepType::Spawn;
	SpawnStep.AffectedTiles.Add(*TargetTile);
	SpawnStep.AffectedPositions.Add(TargetPosition);
	SpawnStep.SnapshotAfterStep = GetSnapshot();
	OutResult.ResolveSteps.Add(SpawnStep);

	if (bResolveAfterUse)
	{
		int32 ResolveScoreDelta = 0;
		int32 ResolveRemovedTileCount = 0;
		int32 ResolveMaxComboIndex = 0;
		ResolveCurrentMatchesIntoSteps(OutResult.ResolveSteps, ResolveScoreDelta, ResolveRemovedTileCount, ResolveMaxComboIndex);
		OutResult.TotalScoreDelta += ResolveScoreDelta;
		OutResult.RemovedTileCount += ResolveRemovedTileCount;
	}

	OutResult.bAccepted = true;
	return true;
}

bool UOrigamiBirdMatchGameObject::ApplyPropSwapColumns(int32 FirstColumn, int32 SecondColumn, bool bResolveAfterUse, FOrigamiBirdPropUseResult& OutResult)
{
	if (!IsInsideColumn(FirstColumn) || !IsInsideColumn(SecondColumn))
	{
		OutResult.FailureReasonId = TEXT("OutOfBoard");
		return false;
	}

	if (FirstColumn == SecondColumn)
	{
		OutResult.FailureReasonId = TEXT("SameColumn");
		return false;
	}

	FOrigamiBirdResolveStep SwapStep;
	SwapStep.StepType = EOrigamiBirdResolveStepType::Swap;

	for (int32 Y = 0; Y < StartParams.BoardHeight; ++Y)
	{
		const FIntPoint FirstPosition(FirstColumn, Y);
		const FIntPoint SecondPosition(SecondColumn, Y);
		const FOrigamiBirdTile* FirstTile = GetTile(FirstPosition);
		const FOrigamiBirdTile* SecondTile = GetTile(SecondPosition);

		if (FirstTile && FirstTile->TileId != INDEX_NONE)
		{
			FOrigamiBirdTileTransition Transition;
			Transition.TileId = FirstTile->TileId;
			Transition.TileType = FirstTile->TileType;
			Transition.FromPosition = FirstPosition;
			Transition.ToPosition = SecondPosition;
			SwapStep.TileTransitions.Add(Transition);
		}

		if (SecondTile && SecondTile->TileId != INDEX_NONE)
		{
			FOrigamiBirdTileTransition Transition;
			Transition.TileId = SecondTile->TileId;
			Transition.TileType = SecondTile->TileType;
			Transition.FromPosition = SecondPosition;
			Transition.ToPosition = FirstPosition;
			SwapStep.TileTransitions.Add(Transition);
		}
	}

	OutResult.ResolveSteps.Add(SwapStep);

	for (int32 Y = 0; Y < StartParams.BoardHeight; ++Y)
	{
		SwapTileData(FIntPoint(FirstColumn, Y), FIntPoint(SecondColumn, Y));
	}

	if (bResolveAfterUse)
	{
		int32 ResolveScoreDelta = 0;
		int32 ResolveRemovedTileCount = 0;
		int32 ResolveMaxComboIndex = 0;
		ResolveCurrentMatchesIntoSteps(OutResult.ResolveSteps, ResolveScoreDelta, ResolveRemovedTileCount, ResolveMaxComboIndex);
		OutResult.TotalScoreDelta += ResolveScoreDelta;
		OutResult.RemovedTileCount += ResolveRemovedTileCount;
	}

	OutResult.bAccepted = true;
	return true;
}

bool UOrigamiBirdMatchGameObject::ApplyPropCopyColumnToNeighbor(int32 SourceColumn, bool bResolveAfterUse, FOrigamiBirdPropUseResult& OutResult)
{
	if (!IsInsideColumn(SourceColumn))
	{
		OutResult.FailureReasonId = TEXT("OutOfBoard");
		return false;
	}

	const int32 TargetColumn = IsInsideColumn(SourceColumn + 1) ? SourceColumn + 1 : SourceColumn - 1;
	if (!IsInsideColumn(TargetColumn))
	{
		OutResult.FailureReasonId = TEXT("NoNeighborColumn");
		return false;
	}

	TArray<FIntPoint> TargetPositions;
	for (int32 Y = 0; Y < StartParams.BoardHeight; ++Y)
	{
		TargetPositions.Add(FIntPoint(TargetColumn, Y));
	}

	FOrigamiBirdResolveStep RemoveStep = MakeRemoveStep(TargetPositions, 0);
	RemoveStep.SnapshotAfterStep = GetSnapshot();
	OutResult.ResolveSteps.Add(RemoveStep);

	FOrigamiBirdResolveStep SpawnStep;
	SpawnStep.StepType = EOrigamiBirdResolveStepType::Spawn;

	for (int32 Y = 0; Y < StartParams.BoardHeight; ++Y)
	{
		const FOrigamiBirdTile* SourceTile = GetTile(FIntPoint(SourceColumn, Y));
		FOrigamiBirdTile* TargetTile = GetTile(FIntPoint(TargetColumn, Y));
		if (!SourceTile || !TargetTile)
		{
			continue;
		}

		TargetTile->TileType = SourceTile->TileType;
		TargetTile->TileId = NextTileId++;
		TargetTile->BoardPosition = FIntPoint(TargetColumn, Y);
		TargetTile->bIsSelected = false;

		SpawnStep.AffectedTiles.Add(*TargetTile);
		SpawnStep.AffectedPositions.Add(TargetTile->BoardPosition);
	}

	SpawnStep.SnapshotAfterStep = GetSnapshot();
	OutResult.ResolveSteps.Add(SpawnStep);

	if (bResolveAfterUse)
	{
		int32 ResolveScoreDelta = 0;
		int32 ResolveRemovedTileCount = 0;
		int32 ResolveMaxComboIndex = 0;
		ResolveCurrentMatchesIntoSteps(OutResult.ResolveSteps, ResolveScoreDelta, ResolveRemovedTileCount, ResolveMaxComboIndex);
		OutResult.TotalScoreDelta += ResolveScoreDelta;
		OutResult.RemovedTileCount += ResolveRemovedTileCount;
	}

	OutResult.bAccepted = true;
	return true;
}

bool UOrigamiBirdMatchGameObject::ApplyPropShuffleBoard(bool bResolveAfterUse, FOrigamiBirdPropUseResult& OutResult)
{
	TArray<FIntPoint> Positions;
	TArray<EOrigamiBirdTileType> TileTypes;

	for (int32 Y = 0; Y < StartParams.BoardHeight; ++Y)
	{
		for (int32 X = 0; X < StartParams.BoardWidth; ++X)
		{
			const FIntPoint Position(X, Y);
			const FOrigamiBirdTile* Tile = GetTile(Position);
			if (Tile && Tile->TileType != EOrigamiBirdTileType::None && CanFallTileType(Tile->TileType))
			{
				Positions.Add(Position);
				TileTypes.Add(Tile->TileType);
			}
		}
	}

	if (Positions.Num() <= 1)
	{
		OutResult.FailureReasonId = TEXT("NotEnoughTiles");
		return false;
	}

	FOrigamiBirdResolveStep RemoveStep = MakeRemoveStep(Positions, 0);
	RemoveStep.SnapshotAfterStep = GetSnapshot();
	OutResult.ResolveSteps.Add(RemoveStep);

	for (int32 Index = TileTypes.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = RandomStream.RandRange(0, Index);
		TileTypes.Swap(Index, SwapIndex);
	}

	FOrigamiBirdResolveStep SpawnStep;
	SpawnStep.StepType = EOrigamiBirdResolveStepType::Spawn;

	for (int32 Index = 0; Index < Positions.Num(); ++Index)
	{
		FOrigamiBirdTile* Tile = GetTile(Positions[Index]);
		if (!Tile)
		{
			continue;
		}

		Tile->TileType = TileTypes[Index];
		Tile->TileId = NextTileId++;
		Tile->BoardPosition = Positions[Index];
		Tile->bIsSelected = false;

		SpawnStep.AffectedTiles.Add(*Tile);
		SpawnStep.AffectedPositions.Add(Tile->BoardPosition);
	}

	SpawnStep.SnapshotAfterStep = GetSnapshot();
	OutResult.ResolveSteps.Add(SpawnStep);

	if (bResolveAfterUse)
	{
		int32 ResolveScoreDelta = 0;
		int32 ResolveRemovedTileCount = 0;
		int32 ResolveMaxComboIndex = 0;
		ResolveCurrentMatchesIntoSteps(OutResult.ResolveSteps, ResolveScoreDelta, ResolveRemovedTileCount, ResolveMaxComboIndex);
		OutResult.TotalScoreDelta += ResolveScoreDelta;
		OutResult.RemovedTileCount += ResolveRemovedTileCount;
	}

	OutResult.bAccepted = true;
	return true;
}

bool UOrigamiBirdMatchGameObject::ApplyPropExplode3x3(FIntPoint CenterPosition, bool bResolveAfterUse, FOrigamiBirdPropUseResult& OutResult)
{
	if (!IsInsideBoard(CenterPosition))
	{
		OutResult.FailureReasonId = TEXT("OutOfBoard");
		return false;
	}

	TArray<FIntPoint> RemovedPositions;
	for (int32 Y = CenterPosition.Y - 1; Y <= CenterPosition.Y + 1; ++Y)
	{
		for (int32 X = CenterPosition.X - 1; X <= CenterPosition.X + 1; ++X)
		{
			const FIntPoint Position(X, Y);
			const FOrigamiBirdTile* Tile = GetTile(Position);
			if (Tile && Tile->TileType != EOrigamiBirdTileType::None && CanFallTileType(Tile->TileType))
			{
				RemovedPositions.Add(Position);
			}
		}
	}

	if (RemovedPositions.IsEmpty())
	{
		OutResult.FailureReasonId = TEXT("NoTilesToRemove");
		return false;
	}

	FOrigamiBirdResolveStep RemoveStep = MakeRemoveStep(RemovedPositions, 0);
	RemoveStep.SnapshotAfterStep = GetSnapshot();
	OutResult.ResolveSteps.Add(RemoveStep);

	RemoveTiles(RemovedPositions);
	RemovedTileCount += RemovedPositions.Num();
	OutResult.RemovedTileCount += RemovedPositions.Num();

	CollapseAndRefill(&OutResult.ResolveSteps);

	if (bResolveAfterUse)
	{
		int32 ResolveScoreDelta = 0;
		int32 ResolveRemovedTileCount = 0;
		int32 ResolveMaxComboIndex = 0;
		ResolveCurrentMatchesIntoSteps(OutResult.ResolveSteps, ResolveScoreDelta, ResolveRemovedTileCount, ResolveMaxComboIndex);
		OutResult.TotalScoreDelta += ResolveScoreDelta;
		OutResult.RemovedTileCount += ResolveRemovedTileCount;
	}

	OutResult.bAccepted = true;
	return true;
}

bool UOrigamiBirdMatchGameObject::CanMatchTileType(EOrigamiBirdTileType TileType) const
{
	if (TileType == EOrigamiBirdTileType::None)
	{
		return false;
	}

	if (const FOrigamiBirdTileDefinitionRow* Definition = FindTileDefinitionInternal(TileType))
	{
		return Definition->bCanMatch;
	}

	return true;
}

bool UOrigamiBirdMatchGameObject::CanFallTileType(EOrigamiBirdTileType TileType) const
{
	if (TileType == EOrigamiBirdTileType::None)
	{
		return true;
	}

	if (const FOrigamiBirdTileDefinitionRow* Definition = FindTileDefinitionInternal(TileType))
	{
		return Definition->bCanFall;
	}

	return true;
}

bool UOrigamiBirdMatchGameObject::CanSwapTileType(EOrigamiBirdTileType TileType) const
{
	if (TileType == EOrigamiBirdTileType::None)
	{
		return false;
	}

	if (const FOrigamiBirdTileDefinitionRow* Definition = FindTileDefinitionInternal(TileType))
	{
		return Definition->bCanSwap;
	}

	return true;
}

int32 UOrigamiBirdMatchGameObject::GetTileScoreValue(EOrigamiBirdTileType TileType) const
{
	if (const FOrigamiBirdTileDefinitionRow* Definition = FindTileDefinitionInternal(TileType))
	{
		return FMath::Max(0, Definition->ScoreValue);
	}

	return 10;
}

bool UOrigamiBirdMatchGameObject::WouldCreateMatchAt(FIntPoint Position) const
{
	//这个函数用于判断在当前位置是否会产生三消，True表示会产生三消，False则表示不会
	const FOrigamiBirdTile* Tile = GetTile(Position);
	if (!Tile || !CanMatchTileType(Tile->TileType))
	{
		return false;
	}

	if (Position.X >= 2)
	{
		const FOrigamiBirdTile* Left1 = GetTile(FIntPoint(Position.X - 1, Position.Y));
		const FOrigamiBirdTile* Left2 = GetTile(FIntPoint(Position.X - 2, Position.Y));

		if (Left1 && Left2
			&& Left1->TileType == Tile->TileType
			&& Left2->TileType == Tile->TileType)
		{
			return true;
		}
	}

	if (Position.Y >= 2)
	{
		const FOrigamiBirdTile* Up1 = GetTile(FIntPoint(Position.X, Position.Y - 1));
		const FOrigamiBirdTile* Up2 = GetTile(FIntPoint(Position.X, Position.Y - 2));

		if (Up1 && Up2
			&& Up1->TileType == Tile->TileType
			&& Up2->TileType == Tile->TileType)
		{
			return true;
		}
	}

	return false;
}

TArray<FIntPoint> UOrigamiBirdMatchGameObject::FindAllMatches() const
{
	TSet<FIntPoint> MatchedSet;

	// 横向扫描
	for (int32 Y = 0; Y < StartParams.BoardHeight; ++Y)
	{
		int32 RunStartX = 0;

		for (int32 X = 1; X <= StartParams.BoardWidth; ++X)
		{
			const EOrigamiBirdTileType RunType = GetTile(FIntPoint(RunStartX, Y))->TileType;

			const bool bContinueRun =
				X < StartParams.BoardWidth
				&& GetTile(FIntPoint(X, Y))->TileType == RunType
				&& CanMatchTileType(RunType);

			if (bContinueRun)
			{
				continue;
			}

			const int32 RunLength = X - RunStartX;

			if (CanMatchTileType(RunType) && RunLength >= 3)
			{
				for (int32 MatchX = RunStartX; MatchX < X; ++MatchX)
				{
					MatchedSet.Add(FIntPoint(MatchX, Y));
				}
			}

			RunStartX = X;
		}
	}

	// 纵向扫描
	for (int32 X = 0; X < StartParams.BoardWidth; ++X)
	{
		int32 RunStartY = 0;

		for (int32 Y = 1; Y <= StartParams.BoardHeight; ++Y)
		{
			const EOrigamiBirdTileType RunType = GetTile(FIntPoint(X, RunStartY))->TileType;

			const bool bContinueRun =
				Y < StartParams.BoardHeight
				&& GetTile(FIntPoint(X, Y))->TileType == RunType
				&& CanMatchTileType(RunType);

			if (bContinueRun)
			{
				continue;
			}

			const int32 RunLength = Y - RunStartY;

			if (CanMatchTileType(RunType) && RunLength >= 3)
			{
				for (int32 MatchY = RunStartY; MatchY < Y; ++MatchY)
				{
					MatchedSet.Add(FIntPoint(X, MatchY));
				}
			}

			RunStartY = Y;
		}
	}

	TArray<FIntPoint> MatchedPositions = MatchedSet.Array();

	MatchedPositions.Sort([](const FIntPoint& A, const FIntPoint& B)
	{
		return A.Y == B.Y ? A.X < B.X : A.Y < B.Y;
	});

	return MatchedPositions;
}

bool UOrigamiBirdMatchGameObject::AreAdjacent(FIntPoint A, FIntPoint B) const
{
	const int32 Distance = FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y);
	return Distance == 1;
}

void UOrigamiBirdMatchGameObject::SwapTileData(FIntPoint A, FIntPoint B)
{
	FOrigamiBirdTile* TileA = GetTile(A);
	FOrigamiBirdTile* TileB = GetTile(B);

	if (!TileA || !TileB)
	{
		return;
	}

	Swap(TileA->TileType, TileB->TileType);
	Swap(TileA->TileId, TileB->TileId);
}

void UOrigamiBirdMatchGameObject::RemoveTiles(const TArray<FIntPoint>& Positions)
{
	for (const FIntPoint& Position : Positions)
	{
		if (FOrigamiBirdTile* Tile = GetTile(Position))
		{
			Tile->TileType = EOrigamiBirdTileType::None;
			Tile->TileId = INDEX_NONE;
			Tile->bIsSelected = false;
		}
	}
}

void UOrigamiBirdMatchGameObject::CollapseAndRefill(TArray<FOrigamiBirdResolveStep>* OutSteps)
{
	// 每一列单独处理。bCanFall=false 的格子会把一列切成多个独立下落区间。
	FOrigamiBirdResolveStep FallStep;
	FallStep.StepType = EOrigamiBirdResolveStepType::Fall;

	FOrigamiBirdResolveStep SpawnStep;
	SpawnStep.StepType = EOrigamiBirdResolveStepType::Spawn;

	for (int32 X = 0; X < StartParams.BoardWidth; ++X)
	{
		int32 SegmentBottomY = StartParams.BoardHeight - 1;

		while (SegmentBottomY >= 0)
		{
			const FOrigamiBirdTile* SegmentBottomTile = GetTile(FIntPoint(X, SegmentBottomY));
			if (SegmentBottomTile
				&& SegmentBottomTile->TileType != EOrigamiBirdTileType::None
				&& !CanFallTileType(SegmentBottomTile->TileType))
			{
				--SegmentBottomY;
				continue;
			}

			int32 SegmentTopY = SegmentBottomY;
			while (SegmentTopY >= 0)
			{
				const FOrigamiBirdTile* Tile = GetTile(FIntPoint(X, SegmentTopY));
				if (Tile && Tile->TileType != EOrigamiBirdTileType::None && !CanFallTileType(Tile->TileType))
				{
					break;
				}

				--SegmentTopY;
			}

			int32 WriteY = SegmentBottomY;
			for (int32 ReadY = SegmentBottomY; ReadY > SegmentTopY; --ReadY)
			{
				FOrigamiBirdTile* ReadTile = GetTile(FIntPoint(X, ReadY));
				if (ReadTile && ReadTile->TileType != EOrigamiBirdTileType::None)
				{
					if (ReadY != WriteY)
					{
						const FIntPoint FromPosition(X, ReadY);
						const FIntPoint ToPosition(X, WriteY);

						FOrigamiBirdTileTransition Transition;
						Transition.TileId = ReadTile->TileId;
						Transition.TileType = ReadTile->TileType;
						Transition.FromPosition = FromPosition;
						Transition.ToPosition = ToPosition;
						FallStep.TileTransitions.Add(Transition);

						FOrigamiBirdTile* WriteTile = GetTile(FIntPoint(X, WriteY));
						WriteTile->TileType = ReadTile->TileType;
						WriteTile->TileId = ReadTile->TileId;
						WriteTile->BoardPosition = ToPosition;
						WriteTile->bIsSelected = false;

						ReadTile->TileType = EOrigamiBirdTileType::None;
						ReadTile->TileId = INDEX_NONE;
						ReadTile->bIsSelected = false;
					}
					else
					{
						ReadTile->bIsSelected = false;
					}

					--WriteY;
				}
			}

			for (int32 Y = WriteY; Y > SegmentTopY; --Y)
			{
				FOrigamiBirdTile* Tile = GetTile(FIntPoint(X, Y));
				Tile->TileType = GenerateRandomTileType();
				Tile->TileId = NextTileId++;
				Tile->BoardPosition = FIntPoint(X, Y);
				Tile->bIsSelected = false;

				SpawnStep.AffectedTiles.Add(*Tile);
				SpawnStep.AffectedPositions.Add(Tile->BoardPosition);
			}

			SegmentBottomY = SegmentTopY - 1;
		}
	}
	
	//注意，需要重新更新一下正确的位置
	for (int32 Y = 0; Y < StartParams.BoardHeight; ++Y)
	{
		for (int32 X = 0; X < StartParams.BoardWidth; ++X)
		{
			if (FOrigamiBirdTile* Tile = GetTile(FIntPoint(X, Y)))
			{
				Tile->BoardPosition = FIntPoint(X, Y);
			}
		}
	}

	if (OutSteps)
	{
		const FOrigamiBirdBoardSnapshot SnapshotAfterCollapse = GetSnapshot();

		if (!FallStep.TileTransitions.IsEmpty())
		{
			FallStep.SnapshotAfterStep = SnapshotAfterCollapse;
			OutSteps->Add(FallStep);
		}

		if (!SpawnStep.AffectedTiles.IsEmpty())
		{
			SpawnStep.SnapshotAfterStep = SnapshotAfterCollapse;
			OutSteps->Add(SpawnStep);
		}
	}
	
}

void UOrigamiBirdMatchGameObject::CheckGameEnd()
{
	const bool bSuccess = Score >= StartParams.TargetScore;
	const bool bFailed = MovesRemaining <= 0;

	if (!bSuccess && !bFailed)
	{
		return;
	}

	Phase = EOrigamiBirdMatchPhase::GameEnded;

	UE_LOG(
		LogOrigamiBirdMatch,
		Log,
		TEXT("GameEnded Success=%d Score=%d UsedMoves=%d Removed=%d MaxCombo=%d"),
		bSuccess,
		Score,
		UsedMoves,
		RemovedTileCount,
		MaxCombo
	);
}

void UOrigamiBirdMatchGameObject::BroadcastBoardChanged()
{
	OnBoardChanged.Broadcast(GetSnapshot());
}

void UOrigamiBirdMatchGameObject::BroadcastPropStacksChanged()
{
	OnPropStacksChanged.Broadcast(PropStacks);
}

EOrigamiBirdTileType UOrigamiBirdMatchGameObject::GenerateRandomTileType()
{
	if (StartParams.AvailableTileTypes.IsEmpty())
	{
		return EOrigamiBirdTileType::RedFruit;
	}

	for (int32 Attempt = 0; Attempt < 20; ++Attempt)
	{
		const int32 Index = RandomStream.RandRange(0, StartParams.AvailableTileTypes.Num() - 1);
		const EOrigamiBirdTileType CandidateType = StartParams.AvailableTileTypes[Index];
		if (CanMatchTileType(CandidateType) && CanFallTileType(CandidateType))
		{
			return CandidateType;
		}
	}

	return StartParams.AvailableTileTypes[0];
}

EOrigamiBirdTileType UOrigamiBirdMatchGameObject::GenerateRandomTileTypeExcept(EOrigamiBirdTileType ExcludedType)
{
	for (int32 Attempt = 0; Attempt < 30; ++Attempt)
	{
		const EOrigamiBirdTileType CandidateType = GenerateRandomTileType();
		if (CandidateType != ExcludedType)
		{
			return CandidateType;
		}
	}

	for (const EOrigamiBirdTileType CandidateType : StartParams.AvailableTileTypes)
	{
		if (CandidateType != ExcludedType && CanMatchTileType(CandidateType) && CanFallTileType(CandidateType))
		{
			return CandidateType;
		}
	}

	return GenerateRandomTileType();
}

void UOrigamiBirdMatchGameObject::GenerateInitialBoard()
{
	//左上角原点，横轴方向往右是X正方向，纵轴方向往下是Y轴正方向
	//需要思考一个问题：初始生成棋盘的时候不能已经有三消了，需要保证棋盘不是直接消除的状态
	BoardTiles.SetNum(StartParams.BoardWidth * StartParams.BoardHeight);

	for (int32 Y = 0; Y < StartParams.BoardHeight; ++Y)
	{
		for (int32 X = 0; X < StartParams.BoardWidth; ++X)
		{
			const FIntPoint Position(X, Y); //构造顺序：（0，0）（1，0）（2，0）...（0，1）（1，1）...

			FOrigamiBirdTile& Tile = BoardTiles[ToIndex(Position)];
			Tile.TileId = NextTileId++;
			Tile.BoardPosition = Position;
			Tile.bIsSelected = false;
			
			int32 Guard = 0;
			do
			{
				Tile.TileType = GenerateRandomTileType();
				++Guard;
			}
			while (WouldCreateMatchAt(Position) && Guard < 50);
		}
	}
}

void UOrigamiBirdMatchGameObject::DumpBoardToLog() const
{
	UE_LOG(
		LogOrigamiBirdMatch,
		Log,
		TEXT("Board %dx%d Score=%d Moves=%d Phase=%d"),
		StartParams.BoardWidth,
		StartParams.BoardHeight,
		Score,
		MovesRemaining,
		static_cast<int32>(Phase)
	);

	for (int32 Y = 0; Y < StartParams.BoardHeight; ++Y)
	{
		FString Line;

		for (int32 X = 0; X < StartParams.BoardWidth; ++X)
		{
			const FOrigamiBirdTile* Tile = GetTile(FIntPoint(X, Y));
			const int32 TypeValue = Tile ? static_cast<int32>(Tile->TileType) : 0;
			Line += FString::Printf(TEXT("%d "), TypeValue);
		}

		UE_LOG(LogOrigamiBirdMatch, Log, TEXT("%s"), *Line);
	}
}

bool UOrigamiBirdMatchGameObject::TrySwapTiles(FIntPoint From, FIntPoint To)
{
	FOrigamiBirdMoveResult IgnoredResult;
	return TrySwapTilesWithResult(From, To, IgnoredResult);
}

bool UOrigamiBirdMatchGameObject::TrySwapTilesWithResult(FIntPoint From, FIntPoint To,
	FOrigamiBirdMoveResult& OutResult)
{
	OutResult = FOrigamiBirdMoveResult();
	OutResult.From = From;
	OutResult.To = To;
	OutResult.InitialSnapshot = GetSnapshot();
	
	if (Phase != EOrigamiBirdMatchPhase::WaitingInput)
	{
		OutResult.FailureReasonId = TEXT("InvalidPhase");
		OutResult.FinalSnapshot = GetSnapshot();
		return false;
	}

	if (!IsInsideBoard(From) || !IsInsideBoard(To))
	{
		OutResult.FailureReasonId = TEXT("OutOfBoard");
		OutResult.FinalSnapshot = GetSnapshot();
		return false;
	}

	if (!AreAdjacent(From, To))
	{
		OutResult.FailureReasonId = TEXT("NotAdjacent");
		OutResult.FinalSnapshot = GetSnapshot();
		return false;
	}
	
	const FOrigamiBirdTile* FromTile = GetTile(From);
	const FOrigamiBirdTile* ToTile = GetTile(To);
	if (!FromTile || !ToTile || !CanSwapTileType(FromTile->TileType) || !CanSwapTileType(ToTile->TileType))
	{
		OutResult.FailureReasonId = TEXT("CannotSwap");
		OutResult.FinalSnapshot = GetSnapshot();
		return false;
	}
	
	//可以正常交换，进行解算
	Phase = EOrigamiBirdMatchPhase::Resolving;
	ClearSelection();
	OutResult.ResolveSteps.Add(MakeSwapStep(From, To));
	SwapTileData(From, To);
	
	//交换完，找匹配
	const TArray<FIntPoint> FirstMatches = FindAllMatches();
	if (FirstMatches.IsEmpty())
	{
		OutResult.ResolveSteps.Add(MakeSwapStep(To, From));
		SwapTileData(From, To);
		Phase = EOrigamiBirdMatchPhase::WaitingInput;
		OutResult.FailureReasonId = TEXT("NoMatch");
		OutResult.FinalSnapshot = GetSnapshot();
		BroadcastBoardChanged();
		return false;
	}
	
	OutResult.bAccepted = true;
	--MovesRemaining;
	++UsedMoves;
	OutResult.UsedMoveDelta = 1; //额外多了一个步骤
	OnMovesChanged.Broadcast(MovesRemaining);
	
	int32 TotalScoreDelta = 0;
	int32 TotalRemovedTileCount = 0;
	int32 ComboIndex = 0;
	ResolveCurrentMatchesIntoSteps(OutResult.ResolveSteps, TotalScoreDelta, TotalRemovedTileCount, ComboIndex);
	
	Phase = EOrigamiBirdMatchPhase::WaitingInput;
	CheckGameEnd();
	OutResult.TotalScoreDelta = TotalScoreDelta;
	OutResult.RemovedTileCount = TotalRemovedTileCount;
	OutResult.MaxCombo = ComboIndex;
	OutResult.FinalSnapshot = GetSnapshot();

	FOrigamiBirdResolveStep FinalStep;
	FinalStep.StepType = EOrigamiBirdResolveStepType::FinalSnapshot;
	FinalStep.SnapshotAfterStep = OutResult.FinalSnapshot;
	OutResult.ResolveSteps.Add(FinalStep);

	BroadcastBoardChanged();
	return true;
}

FOrigamiBirdTile UOrigamiBirdMatchGameObject::MakeTileSnapshot(FIntPoint Position) const
{
	if (const FOrigamiBirdTile* Tile = GetTile(Position))
	{
		return *Tile;
	}

	FOrigamiBirdTile EmptyTile;
	EmptyTile.BoardPosition = Position;
	return EmptyTile;
}

TArray<FOrigamiBirdTile> UOrigamiBirdMatchGameObject::MakeTileSnapshots(const TArray<FIntPoint>& Positions) const
{
	TArray<FOrigamiBirdTile> Result;
	Result.Reserve(Positions.Num());

	for (const FIntPoint& Position : Positions)
	{
		Result.Add(MakeTileSnapshot(Position));
	}

	return Result;
}

FOrigamiBirdResolveStep UOrigamiBirdMatchGameObject::MakeSwapStep(FIntPoint From, FIntPoint To) const
{
	FOrigamiBirdResolveStep Step;
	Step.StepType = EOrigamiBirdResolveStepType::Swap;

	const FOrigamiBirdTile* FromTile = GetTile(From);
	const FOrigamiBirdTile* ToTile = GetTile(To);

	if (FromTile)
	{
		FOrigamiBirdTileTransition Transition;
		Transition.TileId = FromTile->TileId;
		Transition.TileType = FromTile->TileType;
		Transition.FromPosition = From;
		Transition.ToPosition = To;
		Step.TileTransitions.Add(Transition);
	}

	if (ToTile)
	{
		FOrigamiBirdTileTransition Transition;
		Transition.TileId = ToTile->TileId;
		Transition.TileType = ToTile->TileType;
		Transition.FromPosition = To;
		Transition.ToPosition = From;
		Step.TileTransitions.Add(Transition);
	}

	return Step;
}

FOrigamiBirdResolveStep UOrigamiBirdMatchGameObject::MakeMatchStep(
	const TArray<FIntPoint>& MatchPositions,
	int32 ComboIndex) const
{
	FOrigamiBirdResolveStep Step;
	Step.StepType = EOrigamiBirdResolveStepType::Match;
	Step.AffectedPositions = MatchPositions; //比如可以高亮显示匹配的位置
	Step.AffectedTiles = MakeTileSnapshots(MatchPositions);
	Step.ComboIndex = ComboIndex;
	Step.RemovedTileCount = MatchPositions.Num();
	return Step;
}

FOrigamiBirdResolveStep UOrigamiBirdMatchGameObject::MakeRemoveStep(
	const TArray<FIntPoint>& MatchPositions,
	int32 ComboIndex) const
{
	FOrigamiBirdResolveStep Step;
	Step.StepType = EOrigamiBirdResolveStepType::Remove;
	Step.AffectedPositions = MatchPositions;
	Step.AffectedTiles = MakeTileSnapshots(MatchPositions);
	Step.ComboIndex = ComboIndex;
	Step.RemovedTileCount = MatchPositions.Num();
	return Step;
}

FOrigamiBirdResolveStep UOrigamiBirdMatchGameObject::MakeScoreStep(
	int32 ScoreDelta,
	int32 ComboIndex,
	int32 InRemovedTileCount) const
{
	FOrigamiBirdResolveStep Step;
	Step.StepType = EOrigamiBirdResolveStepType::Score;
	Step.ScoreDelta = ScoreDelta;
	Step.ComboIndex = ComboIndex;
	Step.RemovedTileCount = InRemovedTileCount;
	Step.SnapshotAfterStep = GetSnapshot();
	return Step;
}

void UOrigamiBirdMatchGameObject::ResolveCurrentMatchesIntoSteps(TArray<FOrigamiBirdResolveStep>& OutSteps, int32& InOutTotalScoreDelta, int32& InOutTotalRemovedTileCount, int32& OutMaxComboIndex)
{
	int32 ComboIndex = 0;
	constexpr int32 MaxResolveIterations = 100;
	int32 ResolveGuard = 0;

	while (true)
	{
		++ResolveGuard;
		if (ResolveGuard > MaxResolveIterations)
		{
			UE_LOG(LogOrigamiBirdMatch, Warning, TEXT("ResolveCurrentMatchesIntoSteps reached max iterations."));
			break;
		}

		const TArray<FIntPoint> Matches = FindAllMatches();
		if (Matches.IsEmpty())
		{
			break;
		}

		++ComboIndex;

		FOrigamiBirdResolveStep MatchStep = MakeMatchStep(Matches, ComboIndex);
		MatchStep.SnapshotAfterStep = GetSnapshot();
		OutSteps.Add(MatchStep);

		FOrigamiBirdResolveStep RemoveStep = MakeRemoveStep(Matches, ComboIndex);
		OutSteps.Add(RemoveStep);

		int32 BaseScore = 0;
		for (const FIntPoint& MatchPosition : Matches)
		{
			const FOrigamiBirdTile* Tile = GetTile(MatchPosition);
			BaseScore += Tile ? GetTileScoreValue(Tile->TileType) : 0;
		}

		const float ComboMultiplier =
			ComboIndex == 1 ? 1.0f :
			ComboIndex == 2 ? 1.2f :
			ComboIndex == 3 ? 1.5f :
			2.0f;

		const int32 ScoreDelta = FMath::RoundToInt(static_cast<float>(BaseScore) * ComboMultiplier);

		RemoveTiles(Matches);
		Score += ScoreDelta;
		InOutTotalScoreDelta += ScoreDelta;
		InOutTotalRemovedTileCount += Matches.Num();
		RemovedTileCount += Matches.Num();
		MaxCombo = FMath::Max(ComboIndex, MaxCombo);

		OnScoreChanged.Broadcast(Score);

		OutSteps.Add(MakeScoreStep(ScoreDelta, ComboIndex, Matches.Num()));
		CollapseAndRefill(&OutSteps);
	}

	OutMaxComboIndex = ComboIndex;
}

bool UOrigamiBirdMatchGameObject::SelectTile(FIntPoint BoardPosition)
{
	if (Phase != EOrigamiBirdMatchPhase::WaitingInput || !IsInsideBoard(BoardPosition))
	{
		return false;
	}

	ClearSelection();

	if (FOrigamiBirdTile* Tile = GetTile(BoardPosition))
	{
		Tile->bIsSelected = true;
		OnTileSelected.Broadcast(BoardPosition);
		BroadcastBoardChanged();
		return true;
	}

	return false;
}

void UOrigamiBirdMatchGameObject::ClearSelection()
{
	for (FOrigamiBirdTile& Tile : BoardTiles)
	{
		Tile.bIsSelected = false;
	}
}

int32 UOrigamiBirdMatchGameObject::FindPropStackIndex(FName PropId) const
{
	if (PropId.IsNone())
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index < PropStacks.Num(); ++Index)
	{
		if (PropStacks[Index].PropId == PropId)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

FOrigamiBirdBoardSnapshot UOrigamiBirdMatchGameObject::GetSnapshot() const
{
	FOrigamiBirdBoardSnapshot Snapshot;
	Snapshot.BoardWidth = StartParams.BoardWidth;
	Snapshot.BoardHeight = StartParams.BoardHeight;
	Snapshot.Tiles = BoardTiles;
	Snapshot.Score = Score;
	Snapshot.MovesRemaining = MovesRemaining;
	Snapshot.MaxCombo = MaxCombo;
	Snapshot.RemovedTileCount = RemovedTileCount;
	Snapshot.Phase = Phase;
	return Snapshot;
}

bool UOrigamiBirdMatchGameObject::SubmitCommand(const FOrigamiBirdMatchCommand& Command)
{
	switch (Command.CommandType)
	{
		case EOrigamiBirdMatchCommandType::SelectTile:
			return SelectTile(Command.From);

		case EOrigamiBirdMatchCommandType::SwapTiles:
			return TrySwapTiles(Command.From, Command.To);

		case EOrigamiBirdMatchCommandType::Restart:
			Initialize(StartParams);
			return true;

		default:
			return false;
	}
}
