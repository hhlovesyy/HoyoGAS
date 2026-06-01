#include "Core/OrigamiBirdTileEffectResolver.h"

DEFINE_LOG_CATEGORY_STATIC(LogOrigamiBirdTileEffectResolver, Log, All);

namespace
{
	const FIntPoint AdjacentOffsets[] =
	{
		FIntPoint(1, 0),
		FIntPoint(-1, 0),
		FIntPoint(0, 1),
		FIntPoint(0, -1)
	};

	void SortPositions(TArray<FIntPoint>& Positions)
	{
		Positions.Sort([](const FIntPoint& A, const FIntPoint& B)
		{
			return A.Y == B.Y ? A.X < B.X : A.Y < B.Y;
		});
	}

	void ExecuteTriggersAtPosition(
		const FOrigamiBirdBoardState& BoardState,
		FIntPoint TriggerPosition,
		FIntPoint OtherPosition,
		EOrigamiBirdTileTriggerType TriggerType,
		const TArray<FIntPoint>& SourcePositions,
		FOrigamiBirdFindTileDefinition FindTileDefinition,
		FOrigamiBirdCanClearTileType CanClearByEffectTileType,
		TSet<int32>& TriggeredTileIndices,
		TArray<FIntPoint>& InOutRemovedPositions)
	{
		const FOrigamiBirdTile* Tile = BoardState.GetTile(TriggerPosition);
		if (!Tile || Tile->TileType == EOrigamiBirdTileType::None)
		{
			return;
		}

		const FOrigamiBirdTileDefinitionRow* Definition = FindTileDefinition(Tile->TileType);
		if (!Definition)
		{
			return;
		}

		for (int32 TriggerIndex = 0; TriggerIndex < Definition->Triggers.Num(); ++TriggerIndex)
		{
			const FOrigamiBirdTileTriggerDefinition& Trigger = Definition->Triggers[TriggerIndex];
			if (Trigger.TriggerType != TriggerType || !Trigger.EffectClass)
			{
				continue;
			}

			const int32 TriggerKey = (BoardState.ToIndex(TriggerPosition) * 257 + static_cast<int32>(Trigger.TriggerType)) * 257 + TriggerIndex;
			if (TriggeredTileIndices.Contains(TriggerKey))
			{
				continue;
			}

			const UOrigamiBirdTileEffect* Effect = Trigger.EffectClass->GetDefaultObject<UOrigamiBirdTileEffect>();
			if (!Effect)
			{
				continue;
			}

			FString ValidationError;
			if (!Effect->ValidateTrigger(Trigger, ValidationError))
			{
				UE_LOG(
					LogOrigamiBirdTileEffectResolver,
					Error,
					TEXT("Tile effect trigger is invalid. TileType=%d TriggerType=%d Error=%s"),
					static_cast<int32>(Tile->TileType),
					static_cast<int32>(Trigger.TriggerType),
					*ValidationError);
				continue;
			}

			FOrigamiBirdTileEffectContext Context(BoardState, TriggerType, TriggerPosition, SourcePositions, CanClearByEffectTileType);
			Context.OtherPosition = OtherPosition;

			FOrigamiBirdTileEffectResult EffectResult;
			if (!Effect->Execute(Context, Trigger, EffectResult))
			{
				continue;
			}

			TriggeredTileIndices.Add(TriggerKey);
			for (const FIntPoint& RemovedPosition : EffectResult.RemovedPositions)
			{
				if (BoardState.IsInsideBoard(RemovedPosition))
				{
					InOutRemovedPositions.AddUnique(RemovedPosition);
				}
			}
		}
	}

	void AddMatchedAndAdjacentTriggers(
		const FOrigamiBirdBoardState& BoardState,
		const TArray<FIntPoint>& MatchPositions,
		const TArray<FIntPoint>& RemovedByMatchPositions,
		FOrigamiBirdFindTileDefinition FindTileDefinition,
		FOrigamiBirdCanClearTileType CanClearByEffectTileType,
		TSet<int32>& TriggeredTileIndices,
		TArray<FIntPoint>& InOutRemovedPositions)
	{
		for (const FIntPoint& MatchPosition : MatchPositions)
		{
			ExecuteTriggersAtPosition(
				BoardState,
				MatchPosition,
				FIntPoint(INDEX_NONE, INDEX_NONE),
				EOrigamiBirdTileTriggerType::Matched,
				MatchPositions,
				FindTileDefinition,
				CanClearByEffectTileType,
				TriggeredTileIndices,
				InOutRemovedPositions);
		}

		const TArray<FIntPoint> RemovedByMatchPositionsSnapshot = RemovedByMatchPositions;
		for (const FIntPoint& RemovedPosition : RemovedByMatchPositionsSnapshot)
		{
			ExecuteTriggersAtPosition(
				BoardState,
				RemovedPosition,
				FIntPoint(INDEX_NONE, INDEX_NONE),
				EOrigamiBirdTileTriggerType::Removed,
				MatchPositions,
				FindTileDefinition,
				CanClearByEffectTileType,
				TriggeredTileIndices,
				InOutRemovedPositions);

			for (const FIntPoint& Offset : AdjacentOffsets)
			{
				const FIntPoint AdjacentPosition = RemovedPosition + Offset;
				if (!BoardState.IsInsideBoard(AdjacentPosition))
				{
					continue;
				}

				ExecuteTriggersAtPosition(
					BoardState,
					AdjacentPosition,
					RemovedPosition,
					EOrigamiBirdTileTriggerType::AdjacentRemoved,
					MatchPositions,
					FindTileDefinition,
					CanClearByEffectTileType,
					TriggeredTileIndices,
					InOutRemovedPositions);
			}
		}
	}
}

TArray<FIntPoint> FOrigamiBirdTileEffectResolver::ExpandMatchedRemovePositions(
	const FOrigamiBirdBoardState& BoardState,
	const TArray<FIntPoint>& MatchPositions,
	FOrigamiBirdFindTileDefinition FindTileDefinition,
	FOrigamiBirdCanClearTileType CanClearByMatchTileType,
	FOrigamiBirdCanClearTileType CanClearByEffectTileType)
{
	TArray<FIntPoint> RemovedPositions;
	for (const FIntPoint& MatchPosition : MatchPositions)
	{
		const FOrigamiBirdTile* Tile = BoardState.GetTile(MatchPosition);
		if (Tile && Tile->TileType != EOrigamiBirdTileType::None && CanClearByMatchTileType(Tile->TileType))
		{
			RemovedPositions.AddUnique(MatchPosition);
		}
	}

	TSet<int32> TriggeredTileIndices;
	AddMatchedAndAdjacentTriggers(
		BoardState,
		MatchPositions,
		RemovedPositions,
		FindTileDefinition,
		CanClearByEffectTileType,
		TriggeredTileIndices,
		RemovedPositions);

	SortPositions(RemovedPositions);
	return RemovedPositions;
}

TArray<FIntPoint> FOrigamiBirdTileEffectResolver::ResolveSwapRemovePositions(
	const FOrigamiBirdBoardState& BoardState,
	FIntPoint From,
	FIntPoint To,
	FOrigamiBirdFindTileDefinition FindTileDefinition,
	FOrigamiBirdCanClearTileType CanClearByEffectTileType)
{
	TArray<FIntPoint> SourcePositions;
	SourcePositions.Add(From);
	SourcePositions.Add(To);

	TArray<FIntPoint> RemovedPositions;
	TSet<int32> TriggeredTileIndices;

	ExecuteTriggersAtPosition(
		BoardState,
		From,
		To,
		EOrigamiBirdTileTriggerType::Swapped,
		SourcePositions,
		FindTileDefinition,
		CanClearByEffectTileType,
		TriggeredTileIndices,
		RemovedPositions);

	ExecuteTriggersAtPosition(
		BoardState,
		To,
		From,
		EOrigamiBirdTileTriggerType::Swapped,
		SourcePositions,
		FindTileDefinition,
		CanClearByEffectTileType,
		TriggeredTileIndices,
		RemovedPositions);

	SortPositions(RemovedPositions);
	return RemovedPositions;
}
