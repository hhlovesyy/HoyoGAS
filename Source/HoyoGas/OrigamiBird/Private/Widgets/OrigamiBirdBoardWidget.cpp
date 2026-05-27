#include "Widgets/OrigamiBirdBoardWidget.h"

#include "Animation/HoyoWidgetTweenLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Engine/Texture2D.h"
#include "Subsystems/OrigamiBirdMatchSubsystem.h"
#include "TimerManager.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/OrigamiBirdTileVisualWidget.h"

namespace
{
	constexpr float SwapStepDuration = 0.14f;
	constexpr float MatchStepDuration = 0.10f;
	constexpr float RemoveStepDuration = 0.14f;
	constexpr float FallStepDuration = 0.18f;
	constexpr float SpawnStepDuration = 0.14f;

	UClass* FindDefaultTileVisualWidgetClass()
	{
		static const FString ExpectedClassName = TEXT("WBP_OrigamiBirdTileVisualWidget_C");

		for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
		{
			UClass* CandidateClass = *ClassIt;
			if (CandidateClass
				&& CandidateClass->IsChildOf(UOrigamiBirdTileVisualWidget::StaticClass())
				&& CandidateClass->GetName().Equals(ExpectedClassName, ESearchCase::IgnoreCase))
			{
				return CandidateClass;
			}
		}

		return UOrigamiBirdTileVisualWidget::StaticClass();
	}
}

void UOrigamiBirdBoardWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureDefaultBoardTree();

	if (!TileVisualWidgetClass)
	{
		TileVisualWidgetClass = FindDefaultTileVisualWidgetClass();
	}
}

void UOrigamiBirdBoardWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResolveStepTimerHandle);
	}

	ClearBoard();
	Super::NativeDestruct();
}

void UOrigamiBirdBoardWidget::BuildFromSnapshot(const FOrigamiBirdBoardSnapshot& Snapshot, UOrigamiBirdMatchSubsystem* InMatchSubsystem)
{
	MatchSubsystem = InMatchSubsystem;
	PendingResolveSteps.Reset();
	PendingResolveStepIndex = 0;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResolveStepTimerHandle);
	}

	ReconcileWithSnapshot(Snapshot);
}

void UOrigamiBirdBoardWidget::PlayMoveResult(const FOrigamiBirdMoveResult& MoveResult)
{
	if (MoveResult.ResolveSteps.IsEmpty())
	{
		if (!MoveResult.FinalSnapshot.Tiles.IsEmpty())
		{
			ReconcileWithSnapshot(MoveResult.FinalSnapshot);
		}
		SetBoardInputEnabled(true);
		return;
	}

	if (!MoveResult.InitialSnapshot.Tiles.IsEmpty())
	{
		ReconcileWithSnapshot(MoveResult.InitialSnapshot);
	}

	PendingResolveSteps = MoveResult.ResolveSteps;
	PendingFinalSnapshot = MoveResult.FinalSnapshot;
	PendingResolveStepIndex = 0;
	TileIdsPendingRemoval.Reset();
	SetBoardInputEnabled(false);
	PlayNextResolveStep();
}

void UOrigamiBirdBoardWidget::ClearBoard()
{
	if (BoardGrid)
	{
		BoardGrid->ClearChildren();
	}

	TileWidgetsById.Reset();
	TileIdByPosition.Reset();
	PendingResolveSteps.Reset();
	PendingResolveStepIndex = 0;
	TileIdsPendingRemoval.Reset();
}

void UOrigamiBirdBoardWidget::SetBoardInputEnabled(bool bInEnabled)
{
	bBoardInputEnabled = bInEnabled;

	for (const TPair<int32, TObjectPtr<UOrigamiBirdTileVisualWidget>>& Pair : TileWidgetsById)
	{
		if (Pair.Value)
		{
			Pair.Value->SetInputEnabled(bInEnabled);
		}
	}
}

void UOrigamiBirdBoardWidget::HandleTileVisualClicked(FIntPoint BoardPosition)
{
	if (!bBoardInputEnabled)
	{
		return;
	}

	OnTileClicked.Broadcast(BoardPosition);
}

void UOrigamiBirdBoardWidget::HandleResolveStepDelayFinished()
{
	if (PendingResolveSteps.IsValidIndex(PendingResolveStepIndex)
		&& PendingResolveSteps[PendingResolveStepIndex].StepType == EOrigamiBirdResolveStepType::Remove)
	{
		for (const int32 TileId : TileIdsPendingRemoval)
		{
			RemoveTileVisual(TileId);
		}
		TileIdsPendingRemoval.Reset();
	}

	++PendingResolveStepIndex;
	PlayNextResolveStep();
}

void UOrigamiBirdBoardWidget::EnsureDefaultBoardTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	BoardGrid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("GeneratedBoardGrid"));
	WidgetTree->RootWidget = BoardGrid;
}

void UOrigamiBirdBoardWidget::RebuildPositionMap()
{
	TileIdByPosition.Reset();

	for (const TPair<int32, TObjectPtr<UOrigamiBirdTileVisualWidget>>& Pair : TileWidgetsById)
	{
		if (Pair.Value)
		{
			TileIdByPosition.Add(Pair.Value->GetBoardPosition(), Pair.Key);
		}
	}
}

void UOrigamiBirdBoardWidget::ReconcileWithSnapshot(const FOrigamiBirdBoardSnapshot& Snapshot)
{
	if (!BoardGrid || !WidgetTree)
	{
		return;
	}

	BoardGrid->ClearChildren();
	TileWidgetsById.Reset();
	TileIdByPosition.Reset();

	for (const FOrigamiBirdTile& Tile : Snapshot.Tiles)
	{
		if (Tile.TileId == INDEX_NONE || Tile.TileType == EOrigamiBirdTileType::None)
		{
			continue;
		}

		if (UOrigamiBirdTileVisualWidget* TileVisual = CreateTileVisual(Tile))
		{
			AddTileVisualToGrid(TileVisual, Tile.BoardPosition);
		}
	}
}

void UOrigamiBirdBoardWidget::PlayNextResolveStep()
{
	if (PendingResolveStepIndex >= PendingResolveSteps.Num())
	{
		FinishResolveSequence();
		return;
	}

	const FOrigamiBirdResolveStep& Step = PendingResolveSteps[PendingResolveStepIndex];
	PlayResolveStep(Step);

	float StepDuration = 0.0f;
	switch (Step.StepType)
	{
	case EOrigamiBirdResolveStepType::Swap:
		StepDuration = SwapStepDuration;
		break;
	case EOrigamiBirdResolveStepType::Match:
		StepDuration = MatchStepDuration;
		break;
	case EOrigamiBirdResolveStepType::Remove:
		StepDuration = RemoveStepDuration;
		break;
	case EOrigamiBirdResolveStepType::Fall:
		StepDuration = FallStepDuration;
		break;
	case EOrigamiBirdResolveStepType::Spawn:
		StepDuration = SpawnStepDuration;
		break;
	default:
		StepDuration = 0.01f;
		break;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ResolveStepTimerHandle, this, &UOrigamiBirdBoardWidget::HandleResolveStepDelayFinished, StepDuration, false);
	}
}

void UOrigamiBirdBoardWidget::PlayResolveStep(const FOrigamiBirdResolveStep& Step)
{
	switch (Step.StepType)
	{
	case EOrigamiBirdResolveStepType::Swap:
	case EOrigamiBirdResolveStepType::Fall:
		for (const FOrigamiBirdTileTransition& Transition : Step.TileTransitions)
		{
			UOrigamiBirdTileVisualWidget* TileVisual = FindTileVisualById(Transition.TileId);
			if (!TileVisual)
			{
				continue;
			}

			MoveTileVisualToGridPosition(TileVisual, Transition.ToPosition);
			UHoyoWidgetTweenLibrary::TweenRenderTranslation(
				TileVisual,
				GetRenderOffsetBetween(Transition.FromPosition, Transition.ToPosition),
				FVector2D::ZeroVector,
				Step.StepType == EOrigamiBirdResolveStepType::Swap ? SwapStepDuration : FallStepDuration,
				EHoyoWidgetTweenEasing::EaseOut,
				2.0f);
		}
		break;

	case EOrigamiBirdResolveStepType::Match:
		for (const FOrigamiBirdTile& Tile : Step.AffectedTiles)
		{
			if (UOrigamiBirdTileVisualWidget* TileVisual = FindTileVisualById(Tile.TileId))
			{
				UHoyoWidgetTweenLibrary::TweenRenderScale(TileVisual, FVector2D(1.0f, 1.0f), FVector2D(1.12f, 1.12f), MatchStepDuration, EHoyoWidgetTweenEasing::EaseOut, 2.0f);
			}
		}
		break;

	case EOrigamiBirdResolveStepType::Remove:
		for (const FOrigamiBirdTile& Tile : Step.AffectedTiles)
		{
			if (UOrigamiBirdTileVisualWidget* TileVisual = FindTileVisualById(Tile.TileId))
			{
				UHoyoWidgetTweenLibrary::TweenRenderScale(TileVisual, FVector2D(1.12f, 1.12f), FVector2D(0.2f, 0.2f), RemoveStepDuration, EHoyoWidgetTweenEasing::EaseIn, 2.0f);
				UHoyoWidgetTweenLibrary::TweenRenderOpacity(TileVisual, 1.0f, 0.0f, RemoveStepDuration, EHoyoWidgetTweenEasing::EaseIn, 2.0f);
				TileIdsPendingRemoval.AddUnique(Tile.TileId);
			}
		}
		break;

	case EOrigamiBirdResolveStepType::Spawn:
		for (const FOrigamiBirdTile& Tile : Step.AffectedTiles)
		{
			if (UOrigamiBirdTileVisualWidget* TileVisual = CreateTileVisual(Tile))
			{
				AddTileVisualToGrid(TileVisual, Tile.BoardPosition);
				TileVisual->SetRenderScale(FVector2D(0.2f, 0.2f));
				TileVisual->SetRenderOpacity(0.0f);
				TileVisual->SetRenderTranslation(FVector2D(0.0f, -TileCellSize));
				UHoyoWidgetTweenLibrary::TweenRenderTranslation(TileVisual, FVector2D(0.0f, -TileCellSize), FVector2D::ZeroVector, SpawnStepDuration, EHoyoWidgetTweenEasing::EaseOut, 2.0f);
				UHoyoWidgetTweenLibrary::TweenRenderScale(TileVisual, FVector2D(0.2f, 0.2f), FVector2D(1.0f, 1.0f), SpawnStepDuration, EHoyoWidgetTweenEasing::EaseOut, 2.0f);
				UHoyoWidgetTweenLibrary::TweenRenderOpacity(TileVisual, 0.0f, 1.0f, SpawnStepDuration, EHoyoWidgetTweenEasing::EaseOut, 2.0f);
			}
		}
		break;

	case EOrigamiBirdResolveStepType::FinalSnapshot:
		if (!Step.SnapshotAfterStep.Tiles.IsEmpty())
		{
			ReconcileWithSnapshot(Step.SnapshotAfterStep);
		}
		break;

	default:
		break;
	}
}

void UOrigamiBirdBoardWidget::FinishResolveSequence()
{
	if (!PendingFinalSnapshot.Tiles.IsEmpty())
	{
		ReconcileWithSnapshot(PendingFinalSnapshot);
	}

	PendingResolveSteps.Reset();
	PendingResolveStepIndex = 0;
	TileIdsPendingRemoval.Reset();
	SetBoardInputEnabled(true);
}

UOrigamiBirdTileVisualWidget* UOrigamiBirdBoardWidget::CreateTileVisual(const FOrigamiBirdTile& Tile)
{
	if (!WidgetTree)
	{
		return nullptr;
	}

	UClass* TileClass = TileVisualWidgetClass.Get();
	if (!TileClass)
	{
		TileClass = UOrigamiBirdTileVisualWidget::StaticClass();
	}

	UOrigamiBirdTileVisualWidget* TileVisual = WidgetTree->ConstructWidget<UOrigamiBirdTileVisualWidget>(TileClass);
	if (!TileVisual)
	{
		return nullptr;
	}

	UTexture2D* IconTexture = nullptr;
	FLinearColor DebugColor = FLinearColor::White;
	ResolveTileVisualData(Tile.TileType, IconTexture, DebugColor);

	TileVisual->OnTileClicked.RemoveDynamic(this, &UOrigamiBirdBoardWidget::HandleTileVisualClicked);
	TileVisual->OnTileClicked.AddDynamic(this, &UOrigamiBirdBoardWidget::HandleTileVisualClicked);
	TileVisual->ApplyTileVisual(Tile, IconTexture, DebugColor);
	TileVisual->SetTileSize(TileCellSize);
	TileVisual->ResetVisualTransform();
	TileVisual->SetInputEnabled(bBoardInputEnabled);
	return TileVisual;
}

UOrigamiBirdTileVisualWidget* UOrigamiBirdBoardWidget::FindTileVisualById(int32 TileId) const
{
	if (const TObjectPtr<UOrigamiBirdTileVisualWidget>* FoundWidget = TileWidgetsById.Find(TileId))
	{
		return FoundWidget->Get();
	}

	return nullptr;
}

void UOrigamiBirdBoardWidget::AddTileVisualToGrid(UOrigamiBirdTileVisualWidget* TileVisual, FIntPoint BoardPosition)
{
	if (!TileVisual || !BoardGrid)
	{
		return;
	}

	TileWidgetsById.Add(TileVisual->GetTileId(), TileVisual);
	TileIdByPosition.Add(BoardPosition, TileVisual->GetTileId());

	if (UUniformGridSlot* GridSlot = BoardGrid->AddChildToUniformGrid(TileVisual, BoardPosition.Y, BoardPosition.X))
	{
		GridSlot->SetHorizontalAlignment(HAlign_Fill);
		GridSlot->SetVerticalAlignment(VAlign_Fill);
	}
}

void UOrigamiBirdBoardWidget::MoveTileVisualToGridPosition(UOrigamiBirdTileVisualWidget* TileVisual, FIntPoint BoardPosition)
{
	if (!TileVisual || !BoardGrid)
	{
		return;
	}

	FOrigamiBirdTile Tile;
	Tile.TileId = TileVisual->GetTileId();
	Tile.TileType = TileVisual->GetTileType();
	Tile.BoardPosition = BoardPosition;
	Tile.bIsSelected = TileVisual->IsSelected();

	UTexture2D* IconTexture = nullptr;
	FLinearColor DebugColor = FLinearColor::White;
	ResolveTileVisualData(Tile.TileType, IconTexture, DebugColor);
	TileVisual->ApplyTileVisual(Tile, IconTexture, DebugColor);

	TileVisual->RemoveFromParent();
	if (UUniformGridSlot* GridSlot = BoardGrid->AddChildToUniformGrid(TileVisual, BoardPosition.Y, BoardPosition.X))
	{
		GridSlot->SetHorizontalAlignment(HAlign_Fill);
		GridSlot->SetVerticalAlignment(VAlign_Fill);
	}

	RebuildPositionMap();
}

void UOrigamiBirdBoardWidget::RemoveTileVisual(int32 TileId)
{
	UOrigamiBirdTileVisualWidget* TileVisual = FindTileVisualById(TileId);
	if (!TileVisual)
	{
		return;
	}

	TileVisual->RemoveFromParent();
	TileWidgetsById.Remove(TileId);
	RebuildPositionMap();
}

void UOrigamiBirdBoardWidget::ResolveTileVisualData(EOrigamiBirdTileType TileType, UTexture2D*& OutIconTexture, FLinearColor& OutDebugColor) const
{
	OutIconTexture = nullptr;
	OutDebugColor = FLinearColor::White;

	if (!MatchSubsystem)
	{
		return;
	}

	FOrigamiBirdTileDefinitionRow TileDefinition;
	if (MatchSubsystem->FindTileDefinition(TileType, TileDefinition))
	{
		OutIconTexture = TileDefinition.Icon.LoadSynchronous();
		OutDebugColor = TileDefinition.DebugColor;
	}
}

FVector2D UOrigamiBirdBoardWidget::GetRenderOffsetBetween(FIntPoint FromPosition, FIntPoint ToPosition) const
{
	return FVector2D(
		static_cast<float>(FromPosition.X - ToPosition.X) * TileCellSize,
		static_cast<float>(FromPosition.Y - ToPosition.Y) * TileCellSize);
}
