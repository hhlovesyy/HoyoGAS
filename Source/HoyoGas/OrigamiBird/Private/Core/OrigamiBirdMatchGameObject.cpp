#include "Core/OrigamiBirdMatchGameObject.h"

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

void UOrigamiBirdMatchGameObject::CollapseAndRefill()
{
	// 每一列单独处理。bCanFall=false 的格子会把一列切成多个独立下落区间。
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
						FOrigamiBirdTile* WriteTile = GetTile(FIntPoint(X, WriteY));
						WriteTile->TileType = ReadTile->TileType;
						WriteTile->TileId = ReadTile->TileId;
						WriteTile->bIsSelected = false;

						ReadTile->TileType = EOrigamiBirdTileType::None;
						ReadTile->TileId = INDEX_NONE;
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
				Tile->bIsSelected = false;
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
	
}

bool UOrigamiBirdMatchGameObject::ResolveBoardAfterValidSwap()
{
	bool bResolvedAnyMatch = false;
	int32 ComboIndex = 0;
	constexpr int32 MaxResolveIterations = 100;
	int32 ResolveGuard = 0;

	while (true)
	{
		++ResolveGuard;
		if (ResolveGuard > MaxResolveIterations)
		{
			UE_LOG(LogOrigamiBirdMatch, Warning, TEXT("ResolveBoardAfterValidSwap reached max iterations."));
			break;
		}

		const TArray<FIntPoint> Matches = FindAllMatches();

		if (Matches.IsEmpty())
		{
			break;
		}

		bResolvedAnyMatch = true;
		++ComboIndex;
		MaxCombo = FMath::Max(ComboIndex, MaxCombo);
		RemovedTileCount += Matches.Num();

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

		Score += FMath::RoundToInt(static_cast<float>(BaseScore) * ComboMultiplier);
		OnScoreChanged.Broadcast(Score);
		
		RemoveTiles(Matches);
		CollapseAndRefill();
	}

	return bResolvedAnyMatch;
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
	if (Phase != EOrigamiBirdMatchPhase::WaitingInput)
	{
		return false;
	}

	if (!IsInsideBoard(From) || !IsInsideBoard(To))
	{
		return false;
	}

	if (!AreAdjacent(From, To))
	{
		return false;
	}

	const FOrigamiBirdTile* FromTile = GetTile(From);
	const FOrigamiBirdTile* ToTile = GetTile(To);
	if (!FromTile || !ToTile || !CanSwapTileType(FromTile->TileType) || !CanSwapTileType(ToTile->TileType))
	{
		return false;
	}

	Phase = EOrigamiBirdMatchPhase::Resolving;
	ClearSelection();

	SwapTileData(From, To);

	const TArray<FIntPoint> Matches = FindAllMatches();

	if (Matches.IsEmpty())
	{
		SwapTileData(From, To);
		Phase = EOrigamiBirdMatchPhase::WaitingInput;
		BroadcastBoardChanged();
		return false;
	}
	
	--MovesRemaining; //扣除有效步数
	++UsedMoves;
	OnMovesChanged.Broadcast(MovesRemaining);
	
	ResolveBoardAfterValidSwap();
	Phase = EOrigamiBirdMatchPhase::WaitingInput;
	CheckGameEnd();
	BroadcastBoardChanged();
	return true;
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
