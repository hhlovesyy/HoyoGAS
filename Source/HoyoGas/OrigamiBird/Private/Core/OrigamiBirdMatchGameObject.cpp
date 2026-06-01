#include "Core/OrigamiBirdMatchGameObject.h"

#include "Core/OrigamiBirdBoardResolver.h"
#include "Core/OrigamiBirdPropEffect.h"
#include "Core/OrigamiBirdTileEffectResolver.h"

//在当前的 C++ 文件（.cpp）中，定义一个专属的自定义日志类别（Log Category）
DEFINE_LOG_CATEGORY_STATIC(LogOrigamiBirdMatch, Log, All);

namespace
{
	bool HasPresentationTimingRule(
		const FOrigamiBirdPresentationConfig& PresentationConfig,
		EOrigamiBirdPresentationEventType EventType)
	{
		for (const FOrigamiBirdPresentationTimingRule& Rule : PresentationConfig.TimingRules)
		{
			if (Rule.EventType == EventType && Rule.Duration > 0.0f)
			{
				return true;
			}
		}

		return false;
	}

	bool ValidatePresentationConfig(const FOrigamiBirdPresentationConfig& PresentationConfig, FString& OutError)
	{
		static const EOrigamiBirdPresentationEventType RequiredEventTypes[] =
		{
			EOrigamiBirdPresentationEventType::Swap,
			EOrigamiBirdPresentationEventType::MatchHighlight,
			EOrigamiBirdPresentationEventType::Remove,
			EOrigamiBirdPresentationEventType::Score,
			EOrigamiBirdPresentationEventType::Fall,
			EOrigamiBirdPresentationEventType::Spawn
		};

		for (const EOrigamiBirdPresentationEventType EventType : RequiredEventTypes)
		{
			if (!HasPresentationTimingRule(PresentationConfig, EventType))
			{
				OutError = FString::Printf(TEXT("missing valid TimingRule for EventType=%d"), static_cast<int32>(EventType));
				return false;
			}
		}

		return true;
	}

	bool ValidatePropUseRequestTargets(
		const FOrigamiBirdPropDefinitionRow& Definition,
		const FOrigamiBirdPropUseRequest& Request,
		FName& OutFailureReasonId)
	{
		switch (Definition.TargetType)
		{
		case EOrigamiBirdPropTargetType::None:
			if (!Request.TargetPositions.IsEmpty() || !Request.TargetColumns.IsEmpty() || !Request.TargetRows.IsEmpty())
			{
				OutFailureReasonId = TEXT("UnexpectedTarget");
				return false;
			}
			return true;

		case EOrigamiBirdPropTargetType::SingleTile:
			if (Request.TargetPositions.Num() != 1 || !Request.TargetColumns.IsEmpty() || !Request.TargetRows.IsEmpty())
			{
				OutFailureReasonId = TEXT("InvalidSingleTileTarget");
				return false;
			}
			return true;

		case EOrigamiBirdPropTargetType::SingleColumn:
			if (Request.TargetColumns.Num() != 1 || !Request.TargetPositions.IsEmpty() || !Request.TargetRows.IsEmpty())
			{
				OutFailureReasonId = TEXT("InvalidSingleColumnTarget");
				return false;
			}
			return true;

		case EOrigamiBirdPropTargetType::TwoColumns:
			if (Request.TargetColumns.Num() != 2 || Request.TargetColumns[0] == Request.TargetColumns[1] || !Request.TargetPositions.IsEmpty() || !Request.TargetRows.IsEmpty())
			{
				OutFailureReasonId = TEXT("InvalidTwoColumnsTarget");
				return false;
			}
			return true;

		default:
			OutFailureReasonId = TEXT("UnsupportedTargetType");
			return false;
		}
	}

	bool ValidateTileDefinitionConfig(const FOrigamiBirdTileDefinitionRow& Definition, FString& OutError)
	{
		if (Definition.TileType == EOrigamiBirdTileType::None)
		{
			OutError = TEXT("TileType must not be None");
			return false;
		}

		if (Definition.CapabilityMask == 0)
		{
			OutError = TEXT("CapabilityMask is empty");
			return false;
		}

		const EOrigamiBirdTileCapability Capabilities = static_cast<EOrigamiBirdTileCapability>(Definition.CapabilityMask);
		if (EnumHasAnyFlags(Capabilities, EOrigamiBirdTileCapability::RandomSpawnable)
			&& (!EnumHasAnyFlags(Capabilities, EOrigamiBirdTileCapability::Matchable)
				|| !EnumHasAnyFlags(Capabilities, EOrigamiBirdTileCapability::AffectedByGravity)))
		{
			OutError = TEXT("RandomSpawnable requires Matchable and AffectedByGravity");
			return false;
		}

		return true;
	}
}

void UOrigamiBirdMatchGameObject::Initialize(const FOrigamiBirdMatchStartParams& InStartParams)
{
	StartParams = InStartParams;

	auto ResetMatchState = [this](EOrigamiBirdMatchPhase InPhase, int32 InMovesRemaining)
	{
		BoardState.Reset();
		PropStacks.Reset();
		NextTileId = 1;
		Score = 0;
		MovesRemaining = InMovesRemaining;
		UsedMoves = 0;
		MaxCombo = 0;
		RemovedTileCount = 0;
		Phase = InPhase;
	};

	auto ResetInitializeFailureState = [this, &ResetMatchState](bool bResetTileDefinitions)
	{
		if (bResetTileDefinitions)
		{
			TileDefinitionsByType.Reset();
		}

		ResetMatchState(EOrigamiBirdMatchPhase::None, 0);
	};

	if (StartParams.AvailableTileTypes.IsEmpty())
	{
		UE_LOG(LogOrigamiBirdMatch, Error, TEXT("Initialize failed: AvailableTileTypes is empty. Check OrigamiBird level configuration."));
		ResetInitializeFailureState(true);
		return;
	}

	FString PresentationConfigError;
	if (!ValidatePresentationConfig(StartParams.PresentationConfig, PresentationConfigError))
	{
		UE_LOG(
			LogOrigamiBirdMatch,
			Error,
			TEXT("Initialize failed: PresentationConfig is invalid: %s. Check OrigamiBird level configuration."),
			*PresentationConfigError);
		ResetInitializeFailureState(true);
		return;
	}

	RebuildTileDefinitionMap();
	for (const FOrigamiBirdTileDefinitionRow& Definition : StartParams.TileDefinitions)
	{
		FString TileDefinitionError;
		if (!ValidateTileDefinitionConfig(Definition, TileDefinitionError))
		{
			UE_LOG(
				LogOrigamiBirdMatch,
				Error,
				TEXT("Initialize failed: TileDefinition for TileType=%d is invalid: %s"),
				static_cast<int32>(Definition.TileType),
				*TileDefinitionError);
			ResetInitializeFailureState(false);
			return;
		}
	}

	bool bHasRandomSpawnableAvailableTileType = false;
	for (const EOrigamiBirdTileType TileType : StartParams.AvailableTileTypes)
	{
		if (TileType == EOrigamiBirdTileType::None || !FindTileDefinitionInternal(TileType))
		{
			UE_LOG(
				LogOrigamiBirdMatch,
				Error,
				TEXT("Initialize failed: AvailableTileTypes contains TileType=%d without a TileDefinition row."),
				static_cast<int32>(TileType));
			ResetInitializeFailureState(false);
			return;
		}

		bHasRandomSpawnableAvailableTileType |= CanGenerateRandomTileType(TileType);
	}

	if (!bHasRandomSpawnableAvailableTileType)
	{
		UE_LOG(
			LogOrigamiBirdMatch,
			Error,
			TEXT("Initialize failed: AvailableTileTypes does not contain any tile type with random generation capabilities."));
		ResetInitializeFailureState(false);
		return;
	}
	StartParams.BoardWidth = FMath::Clamp(StartParams.BoardWidth, 3, 12);
	StartParams.BoardHeight = FMath::Clamp(StartParams.BoardHeight, 3, 12);
	RandomStream.Initialize(StartParams.RandomSeed);

	ResetMatchState(EOrigamiBirdMatchPhase::WaitingInput, FMath::Max(1, StartParams.MoveLimit));

	FOrigamiBirdBoardResolver::GenerateInitialBoard(
		BoardState,
		StartParams.BoardWidth,
		StartParams.BoardHeight,
		[this]()
		{
			return GenerateRandomTileType();
		},
		[this]()
		{
			return NextTileId++;
		},
		[this](EOrigamiBirdTileType TileType)
		{
			return CanMatchTileType(TileType);
		});
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

const FOrigamiBirdTileDefinitionRow* UOrigamiBirdMatchGameObject::FindTileDefinitionInternal(EOrigamiBirdTileType TileType) const
{
	return TileDefinitionsByType.Find(TileType);
}

const FOrigamiBirdTileDefinitionRow* UOrigamiBirdMatchGameObject::FindTileDefinitionForQuery(
	EOrigamiBirdTileType TileType,
	const TCHAR* QueryName) const
{
	if (const FOrigamiBirdTileDefinitionRow* Definition = FindTileDefinitionInternal(TileType))
	{
		return Definition;
	}

	UE_LOG(
		LogOrigamiBirdMatch,
		Error,
		TEXT("Missing TileDefinition for %s TileType=%d."),
		QueryName,
		static_cast<int32>(TileType));
	return nullptr;
}

bool UOrigamiBirdMatchGameObject::HasTileCapability(
	EOrigamiBirdTileType TileType,
	EOrigamiBirdTileCapability Capability) const
{
	const FOrigamiBirdTileDefinitionRow* Definition = FindTileDefinitionForQuery(TileType, TEXT("capability query"));
	if (!Definition)
	{
		return false;
	}

	return EnumHasAnyFlags(static_cast<EOrigamiBirdTileCapability>(Definition->CapabilityMask), Capability);
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

bool UOrigamiBirdMatchGameObject::UsePropWithResult(const FOrigamiBirdPropDefinitionRow& Definition, const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdActionResult& OutResult)
{
	OutResult = FOrigamiBirdActionResult();
	OutResult.ActionType = EOrigamiBirdActionType::UseProp;
	OutResult.PropId = Request.PropId;
	OutResult.InitialSnapshot = GetSnapshot();
	OutResult.PresentationConfig = StartParams.PresentationConfig;

	auto RejectAction = [this, &OutResult](FName FailureReasonId)
	{
		OutResult.FailureReasonId = FailureReasonId;
		OutResult.FinalSnapshot = GetSnapshot();
		return false;
	};

	if (Request.PropId.IsNone())
	{
		return RejectAction(TEXT("InvalidPropId"));
	}

	if (Phase != EOrigamiBirdMatchPhase::WaitingInput)
	{
		return RejectAction(TEXT("InvalidPhase"));
	}

	if (GetPropCount(Request.PropId) <= 0)
	{
		return RejectAction(TEXT("PropNotOwned"));
	}

	if (!Definition.EffectClass)
	{
		return RejectAction(TEXT("MissingEffectClass"));
	}

	const UOrigamiBirdPropEffect* Effect = Definition.EffectClass->GetDefaultObject<UOrigamiBirdPropEffect>();
	if (!Effect)
	{
		return RejectAction(TEXT("InvalidEffectClass"));
	}

	FString PropDefinitionError;
	if (!Effect->ValidateDefinition(Definition, PropDefinitionError))
	{
		UE_LOG(
			LogOrigamiBirdMatch,
			Error,
			TEXT("UsePropWithResult failed because prop '%s' configuration is invalid: %s"),
			*Request.PropId.ToString(),
			*PropDefinitionError);
		return RejectAction(TEXT("InvalidPropDefinition"));
	}

	FName TargetFailureReasonId;
	if (!ValidatePropUseRequestTargets(Definition, Request, TargetFailureReasonId))
	{
		return RejectAction(TargetFailureReasonId);
	}

	Phase = EOrigamiBirdMatchPhase::Resolving;
	BoardState.ClearSelection();

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

	BroadcastBoardChanged();
	return true;
}

bool UOrigamiBirdMatchGameObject::CanMatchTileType(EOrigamiBirdTileType TileType) const
{
	if (TileType == EOrigamiBirdTileType::None)
	{
		return false;
	}

	return HasTileCapability(TileType, EOrigamiBirdTileCapability::Matchable);
}

bool UOrigamiBirdMatchGameObject::CanFallTileType(EOrigamiBirdTileType TileType) const
{
	if (TileType == EOrigamiBirdTileType::None)
	{
		return true;
	}

	return HasTileCapability(TileType, EOrigamiBirdTileCapability::AffectedByGravity);
}

bool UOrigamiBirdMatchGameObject::CanSwapTileType(EOrigamiBirdTileType TileType) const
{
	if (TileType == EOrigamiBirdTileType::None)
	{
		return false;
	}

	return HasTileCapability(TileType, EOrigamiBirdTileCapability::Swappable);
}

bool UOrigamiBirdMatchGameObject::CanClearByMatchTileType(EOrigamiBirdTileType TileType) const
{
	if (TileType == EOrigamiBirdTileType::None)
	{
		return false;
	}

	return HasTileCapability(TileType, EOrigamiBirdTileCapability::ClearableByMatch);
}

bool UOrigamiBirdMatchGameObject::CanClearByEffectTileType(EOrigamiBirdTileType TileType) const
{
	if (TileType == EOrigamiBirdTileType::None)
	{
		return false;
	}

	return HasTileCapability(TileType, EOrigamiBirdTileCapability::ClearableByEffect);
}

bool UOrigamiBirdMatchGameObject::CanGenerateRandomTileType(EOrigamiBirdTileType TileType) const
{
	if (TileType == EOrigamiBirdTileType::None)
	{
		return false;
	}

	const FOrigamiBirdTileDefinitionRow* Definition = FindTileDefinitionForQuery(TileType, TEXT("random generation query"));
	if (!Definition)
	{
		return false;
	}

	const EOrigamiBirdTileCapability Capabilities = static_cast<EOrigamiBirdTileCapability>(Definition->CapabilityMask);
	return EnumHasAnyFlags(Capabilities, EOrigamiBirdTileCapability::RandomSpawnable)
		&& EnumHasAnyFlags(Capabilities, EOrigamiBirdTileCapability::Matchable)
		&& EnumHasAnyFlags(Capabilities, EOrigamiBirdTileCapability::AffectedByGravity);
}

int32 UOrigamiBirdMatchGameObject::GetTileScoreValue(EOrigamiBirdTileType TileType) const
{
	if (TileType == EOrigamiBirdTileType::None)
	{
		return 0;
	}

	const FOrigamiBirdTileDefinitionRow* Definition = FindTileDefinitionForQuery(TileType, TEXT("score query"));
	if (!Definition)
	{
		return 0;
	}

	if (!EnumHasAnyFlags(static_cast<EOrigamiBirdTileCapability>(Definition->CapabilityMask), EOrigamiBirdTileCapability::Scoreable))
	{
		return 0;
	}

	return FMath::Max(0, Definition->ScoreValue);
}

bool UOrigamiBirdMatchGameObject::AreAdjacent(FIntPoint A, FIntPoint B) const
{
	const int32 Distance = FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y);
	return Distance == 1;
}

FOrigamiBirdCollapseAndRefillResult UOrigamiBirdMatchGameObject::CollapseAndRefill()
{
	return FOrigamiBirdBoardResolver::CollapseAndRefill(
		BoardState,
		[this](EOrigamiBirdTileType TileType)
		{
			return CanFallTileType(TileType);
		},
		[this]()
		{
			return GenerateRandomTileType();
		},
		[this]()
		{
			return NextTileId++;
		});
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
		UE_LOG(LogOrigamiBirdMatch, Error, TEXT("GenerateRandomTileType failed: AvailableTileTypes is empty."));
		return EOrigamiBirdTileType::None;
	}

	for (int32 Attempt = 0; Attempt < 20; ++Attempt)
	{
		const int32 Index = RandomStream.RandRange(0, StartParams.AvailableTileTypes.Num() - 1);
		const EOrigamiBirdTileType CandidateType = StartParams.AvailableTileTypes[Index];
		if (CanGenerateRandomTileType(CandidateType))
		{
			return CandidateType;
		}
	}

	for (const EOrigamiBirdTileType CandidateType : StartParams.AvailableTileTypes)
	{
		if (CanGenerateRandomTileType(CandidateType))
		{
			return CandidateType;
		}
	}

	UE_LOG(LogOrigamiBirdMatch, Error, TEXT("GenerateRandomTileType failed: no available tile type satisfies random generation capabilities."));
	return EOrigamiBirdTileType::None;
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
		if (CandidateType != ExcludedType && CanGenerateRandomTileType(CandidateType))
		{
			return CandidateType;
		}
	}

	return GenerateRandomTileType();
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
			const FOrigamiBirdTile* Tile = BoardState.GetTile(FIntPoint(X, Y));
			const int32 TypeValue = Tile ? static_cast<int32>(Tile->TileType) : 0;
			Line += FString::Printf(TEXT("%d "), TypeValue);
		}

		UE_LOG(LogOrigamiBirdMatch, Log, TEXT("%s"), *Line);
	}
}

bool UOrigamiBirdMatchGameObject::TrySwapTiles(FIntPoint From, FIntPoint To)
{
	FOrigamiBirdActionResult IgnoredResult;
	return TrySwapTilesWithResult(From, To, IgnoredResult);
}

bool UOrigamiBirdMatchGameObject::TrySwapTilesWithResult(FIntPoint From, FIntPoint To,
	FOrigamiBirdActionResult& OutResult)
{
	OutResult = FOrigamiBirdActionResult();
	OutResult.ActionType = EOrigamiBirdActionType::SwapTiles;
	OutResult.From = From;
	OutResult.To = To;
	OutResult.InitialSnapshot = GetSnapshot();
	OutResult.PresentationConfig = StartParams.PresentationConfig;

	auto RejectAction = [this, &OutResult](FName FailureReasonId)
	{
		OutResult.FailureReasonId = FailureReasonId;
		OutResult.FinalSnapshot = GetSnapshot();
		return false;
	};

	auto ConsumeMove = [this, &OutResult]()
	{
		OutResult.bAccepted = true;
		--MovesRemaining;
		++UsedMoves;
		OutResult.UsedMoveDelta = 1;
		OnMovesChanged.Broadcast(MovesRemaining);
	};

	auto ApplyResolveResult = [&OutResult](const FOrigamiBirdMatchResolveResult& ResolveResult)
	{
		OutResult.ResolveCycles = ResolveResult.ResolveCycles;
		OutResult.TotalScoreDelta = ResolveResult.TotalScoreDelta;
		OutResult.RemovedTileCount = ResolveResult.RemovedTileCount;
		OutResult.MaxCombo = ResolveResult.MaxComboIndex;
	};

	auto FinalizeAcceptedResolve = [this, &OutResult, &ApplyResolveResult](const FOrigamiBirdMatchResolveResult& ResolveResult)
	{
		Phase = EOrigamiBirdMatchPhase::WaitingInput;
		CheckGameEnd();
		ApplyResolveResult(ResolveResult);
		OutResult.FinalSnapshot = GetSnapshot();
		BroadcastBoardChanged();
		return true;
	};
	
	if (Phase != EOrigamiBirdMatchPhase::WaitingInput)
	{
		return RejectAction(TEXT("InvalidPhase"));
	}

	if (!BoardState.IsInsideBoard(From) || !BoardState.IsInsideBoard(To))
	{
		return RejectAction(TEXT("OutOfBoard"));
	}

	if (!AreAdjacent(From, To))
	{
		return RejectAction(TEXT("NotAdjacent"));
	}
	
	const FOrigamiBirdTile* FromTile = BoardState.GetTile(From);
	const FOrigamiBirdTile* ToTile = BoardState.GetTile(To);
	if (!FromTile || !ToTile || !CanSwapTileType(FromTile->TileType) || !CanSwapTileType(ToTile->TileType))
	{
		return RejectAction(TEXT("CannotSwap"));
	}
	
	//可以正常交换，进行解算
	Phase = EOrigamiBirdMatchPhase::Resolving;
	BoardState.ClearSelection();
	OutResult.BoardChangeSteps.Add(MakeSwapStep(From, To));
	BoardState.SwapTileData(From, To);
	
	//交换完，找匹配
	const TArray<FIntPoint> FirstMatches = FOrigamiBirdBoardResolver::FindAllMatches(
		BoardState,
		[this](EOrigamiBirdTileType TileType)
		{
			return CanMatchTileType(TileType);
		});
	if (FirstMatches.IsEmpty())
	{
		const TArray<FIntPoint> SwapEffectRemovedPositions =
			FOrigamiBirdTileEffectResolver::ResolveSwapRemovePositions(
				BoardState,
				From,
				To,
				[this](EOrigamiBirdTileType TileType)
				{
					return FindTileDefinitionInternal(TileType);
				},
				[this](EOrigamiBirdTileType TileType)
				{
					return CanClearByEffectTileType(TileType);
				});

		if (SwapEffectRemovedPositions.IsEmpty())
		{
			OutResult.BoardChangeSteps.Add(MakeSwapStep(To, From));
			BoardState.SwapTileData(From, To);
			Phase = EOrigamiBirdMatchPhase::WaitingInput;
			BroadcastBoardChanged();
			return RejectAction(TEXT("NoMatch"));
		}

		ConsumeMove();

		FOrigamiBirdResolveSeed SwapEffectSeed;
		SwapEffectSeed.MatchPositions.Add(From);
		SwapEffectSeed.MatchPositions.Add(To);
		SwapEffectSeed.RemovedPositions = SwapEffectRemovedPositions;

		const FOrigamiBirdMatchResolveResult ResolveResult = ResolveCurrentMatches(&SwapEffectSeed);
		return FinalizeAcceptedResolve(ResolveResult);
	}
	
	ConsumeMove();
	
	const FOrigamiBirdMatchResolveResult ResolveResult = ResolveCurrentMatches();
	return FinalizeAcceptedResolve(ResolveResult);
}

FOrigamiBirdTile UOrigamiBirdMatchGameObject::MakeTileSnapshot(FIntPoint Position) const
{
	if (const FOrigamiBirdTile* Tile = BoardState.GetTile(Position))
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

FOrigamiBirdBoardChangeStep UOrigamiBirdMatchGameObject::MakeSwapStep(FIntPoint From, FIntPoint To) const
{
	FOrigamiBirdBoardChangeStep Step;
	Step.StepType = EOrigamiBirdBoardChangeStepType::Swap;

	const FOrigamiBirdTile* FromTile = BoardState.GetTile(From);
	const FOrigamiBirdTile* ToTile = BoardState.GetTile(To);

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

FOrigamiBirdBoardChangeStep UOrigamiBirdMatchGameObject::MakeRemoveStep(
	const TArray<FIntPoint>& Positions) const
{
	FOrigamiBirdBoardChangeStep Step;
	Step.StepType = EOrigamiBirdBoardChangeStepType::Remove;
	Step.AffectedPositions = Positions;
	Step.AffectedTiles = MakeTileSnapshots(Positions);
	Step.SnapshotAfterStep = GetSnapshot();
	return Step;
}

FOrigamiBirdBoardChangeStep UOrigamiBirdMatchGameObject::MakeSpawnStep(
	const TArray<FOrigamiBirdTile>& SpawnedTiles,
	const TArray<FIntPoint>& SpawnedPositions) const
{
	FOrigamiBirdBoardChangeStep Step;
	Step.StepType = EOrigamiBirdBoardChangeStepType::Spawn;
	Step.AffectedTiles = SpawnedTiles;
	Step.AffectedPositions = SpawnedPositions;
	Step.SnapshotAfterStep = GetSnapshot();
	return Step;
}

FOrigamiBirdBoardChangeStep UOrigamiBirdMatchGameObject::MakeFallStep(const TArray<FOrigamiBirdTileTransition>& FallTransitions) const
{
	FOrigamiBirdBoardChangeStep Step;
	Step.StepType = EOrigamiBirdBoardChangeStepType::Fall;
	Step.TileTransitions = FallTransitions;
	Step.SnapshotAfterStep = GetSnapshot();
	return Step;
}

void UOrigamiBirdMatchGameObject::AppendCollapseSteps(
	const FOrigamiBirdCollapseAndRefillResult& CollapseResult,
	FOrigamiBirdActionResult& OutResult) const
{
	if (!CollapseResult.FallTransitions.IsEmpty())
	{
		OutResult.BoardChangeSteps.Add(MakeFallStep(CollapseResult.FallTransitions));
	}

	if (!CollapseResult.SpawnedTiles.IsEmpty())
	{
		OutResult.BoardChangeSteps.Add(MakeSpawnStep(CollapseResult.SpawnedTiles, CollapseResult.SpawnedPositions));
	}
}

void UOrigamiBirdMatchGameObject::ResolveAfterPropUse(FOrigamiBirdActionResult& OutResult)
{
	const FOrigamiBirdMatchResolveResult ResolveResult = ResolveCurrentMatches();
	OutResult.ResolveCycles.Append(ResolveResult.ResolveCycles);
	OutResult.TotalScoreDelta += ResolveResult.TotalScoreDelta;
	OutResult.RemovedTileCount += ResolveResult.RemovedTileCount;
}

FOrigamiBirdMatchResolveResult UOrigamiBirdMatchGameObject::ResolveCurrentMatches(const FOrigamiBirdResolveSeed* InitialSeed)
{
	return FOrigamiBirdMatchResolver::ResolveCurrentMatches(
		BoardState,
		[this](EOrigamiBirdTileType TileType)
		{
			return CanMatchTileType(TileType);
		},
		[this](EOrigamiBirdTileType TileType)
		{
			return CanFallTileType(TileType);
		},
		[this]()
		{
			return GenerateRandomTileType();
		},
		[this]()
		{
			return NextTileId++;
		},
		[this](EOrigamiBirdTileType TileType)
		{
			return GetTileScoreValue(TileType);
		},
		[this](const FOrigamiBirdBoardState& InBoardState, const TArray<FIntPoint>& MatchPositions)
		{
			return FOrigamiBirdTileEffectResolver::ExpandMatchedRemovePositions(
				InBoardState,
				MatchPositions,
				[this](EOrigamiBirdTileType TileType)
				{
					return FindTileDefinitionInternal(TileType);
				},
				[this](EOrigamiBirdTileType TileType)
				{
					return CanClearByMatchTileType(TileType);
				},
				[this](EOrigamiBirdTileType TileType)
				{
					return CanClearByEffectTileType(TileType);
				});
		},
		[this]()
		{
			return GetSnapshot();
		},
		[this](int32 ScoreDelta, int32 InRemovedTileCount, int32 ComboIndex)
		{
			Score += ScoreDelta;
			RemovedTileCount += InRemovedTileCount;
			MaxCombo = FMath::Max(ComboIndex, MaxCombo);
			OnScoreChanged.Broadcast(Score);
		},
		InitialSeed);
}

bool UOrigamiBirdMatchGameObject::SelectTile(FIntPoint BoardPosition)
{
	if (Phase != EOrigamiBirdMatchPhase::WaitingInput || !BoardState.IsInsideBoard(BoardPosition))
	{
		return false;
	}

	BoardState.ClearSelection();

	if (FOrigamiBirdTile* Tile = BoardState.GetTile(BoardPosition))
	{
		Tile->bIsSelected = true;
		OnTileSelected.Broadcast(BoardPosition);
		BroadcastBoardChanged();
		return true;
	}

	return false;
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
	Snapshot.BoardWidth = BoardState.GetWidth();
	Snapshot.BoardHeight = BoardState.GetHeight();
	Snapshot.Tiles = BoardState.GetTiles();
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
