#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdBoardResolver.h"
#include "Core/OrigamiBirdMatchTypes.h"
#include "UObject/Object.h"
#include "OrigamiBirdTileEffect.generated.h"

struct HOYOGAS_API FOrigamiBirdTileEffectContext
{
	FOrigamiBirdTileEffectContext(
		const FOrigamiBirdBoardState& InBoardState,
		EOrigamiBirdTileTriggerType InTriggerType,
		FIntPoint InTriggerPosition,
		const TArray<FIntPoint>& InSourcePositions,
		FOrigamiBirdCanFallTileType InCanRemoveTileType)
		: BoardState(InBoardState)
		, TriggerType(InTriggerType)
		, TriggerPosition(InTriggerPosition)
		, SourcePositions(InSourcePositions)
		, CanRemoveTileType(InCanRemoveTileType)
	{
	}

	const FOrigamiBirdBoardState& BoardState;
	EOrigamiBirdTileTriggerType TriggerType = EOrigamiBirdTileTriggerType::None;
	FIntPoint TriggerPosition = FIntPoint(INDEX_NONE, INDEX_NONE);
	FIntPoint OtherPosition = FIntPoint(INDEX_NONE, INDEX_NONE);
	const TArray<FIntPoint>& SourcePositions;
	FOrigamiBirdCanFallTileType CanRemoveTileType;
};

struct HOYOGAS_API FOrigamiBirdTileEffectResult
{
	TArray<FIntPoint> RemovedPositions;

	void AddRemovedPosition(FIntPoint Position)
	{
		RemovedPositions.AddUnique(Position);
	}
};

UCLASS(Abstract, BlueprintType, Blueprintable)
class HOYOGAS_API UOrigamiBirdTileEffect : public UObject
{
	GENERATED_BODY()

public:
	virtual bool ValidateTrigger(const FOrigamiBirdTileTriggerDefinition& Trigger, FString& OutError) const;
	virtual bool Execute(
		const FOrigamiBirdTileEffectContext& Context,
		const FOrigamiBirdTileTriggerDefinition& Trigger,
		FOrigamiBirdTileEffectResult& InOutResult) const;

protected:
	bool TryGetParamString(const FOrigamiBirdTileTriggerDefinition& Trigger, FName Key, FString& OutValue) const;
	bool TryGetBoolParam(const FOrigamiBirdTileTriggerDefinition& Trigger, FName Key, bool& OutValue) const;
	bool TryGetIntParam(const FOrigamiBirdTileTriggerDefinition& Trigger, FName Key, int32& OutValue) const;
	bool TryGetNameParam(const FOrigamiBirdTileTriggerDefinition& Trigger, FName Key, FName& OutValue) const;
};

// Generic clear effect used by special tiles.
// Supported ClearMode values: Row, Column, Board, Radius3x3, Cross, SameType.
UCLASS(BlueprintType, Blueprintable)
class HOYOGAS_API UOrigamiBirdClearTilesByModeEffect : public UOrigamiBirdTileEffect
{
	GENERATED_BODY()

public:
	virtual bool ValidateTrigger(const FOrigamiBirdTileTriggerDefinition& Trigger, FString& OutError) const override;
	virtual bool Execute(
		const FOrigamiBirdTileEffectContext& Context,
		const FOrigamiBirdTileTriggerDefinition& Trigger,
		FOrigamiBirdTileEffectResult& InOutResult) const override;
};
