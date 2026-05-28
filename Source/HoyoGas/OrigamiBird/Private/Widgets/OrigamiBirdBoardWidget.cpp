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
	constexpr float DefaultEventDuration = 0.01f;
	constexpr float EventStartTimeTolerance = 0.001f;

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
		World->GetTimerManager().ClearTimer(PresentationEventTimerHandle);
	}

	ClearBoard();
	Super::NativeDestruct();
}

void UOrigamiBirdBoardWidget::BuildFromSnapshot(const FOrigamiBirdBoardSnapshot& Snapshot, UOrigamiBirdMatchSubsystem* InMatchSubsystem)
{
	MatchSubsystem = InMatchSubsystem;
	PendingPresentationTimeline = FOrigamiBirdPresentationTimeline();
	PendingPresentationEventIndex = 0;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PresentationEventTimerHandle);
	}

	ReconcileWithSnapshot(Snapshot);
}

void UOrigamiBirdBoardWidget::PlayActionResult(const FOrigamiBirdActionResult& ActionResult)
{
	PlayPresentationTimeline(ActionResult.PresentationTimeline, ActionResult.InitialSnapshot);
}

void UOrigamiBirdBoardWidget::PlayPresentationTimeline(
	const FOrigamiBirdPresentationTimeline& Timeline,
	const FOrigamiBirdBoardSnapshot& InitialSnapshot)
{
	if (!InitialSnapshot.Tiles.IsEmpty())
	{
		ReconcileWithSnapshot(InitialSnapshot);
	}

	if (Timeline.Events.IsEmpty())
	{
		if (!Timeline.FinalSnapshot.Tiles.IsEmpty())
		{
			ReconcileWithSnapshot(Timeline.FinalSnapshot);
		}
		SetBoardInputEnabled(true);
		return;
	}

	PendingPresentationTimeline = Timeline;
	PendingPresentationEventIndex = 0;
	TileIdsPendingRemoval.Reset();
	SetBoardInputEnabled(false);
	PlayNextPresentationEvent();
}

void UOrigamiBirdBoardWidget::ClearBoard()
{
	if (BoardGrid)
	{
		BoardGrid->ClearChildren();
	}

	TileWidgetsById.Reset();
	TileIdByPosition.Reset();
	PendingPresentationTimeline = FOrigamiBirdPresentationTimeline();
	PendingPresentationEventIndex = 0;
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

void UOrigamiBirdBoardWidget::HandlePresentationEventDelayFinished()
{
	for (const int32 TileId : TileIdsPendingRemoval)
	{
		RemoveTileVisual(TileId);
	}
	TileIdsPendingRemoval.Reset();

	PlayNextPresentationEvent();
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

void UOrigamiBirdBoardWidget::PlayNextPresentationEvent()
{
	if (PendingPresentationEventIndex >= PendingPresentationTimeline.Events.Num())
	{
		FinishPresentationTimeline();
		return;
	}

	const float BatchStartTime = PendingPresentationTimeline.Events[PendingPresentationEventIndex].StartTime;
	float BatchDuration = 0.0f;

	while (PendingPresentationTimeline.Events.IsValidIndex(PendingPresentationEventIndex)
		&& FMath::IsNearlyEqual(
			PendingPresentationTimeline.Events[PendingPresentationEventIndex].StartTime,
			BatchStartTime,
			EventStartTimeTolerance))
	{
		const FOrigamiBirdPresentationEvent& Event = PendingPresentationTimeline.Events[PendingPresentationEventIndex];
		PlayPresentationEvent(Event);
		BatchDuration = FMath::Max(BatchDuration, Event.Duration);
		++PendingPresentationEventIndex;
	}

	float Delay = FMath::Max(BatchDuration, DefaultEventDuration);
	if (PendingPresentationTimeline.Events.IsValidIndex(PendingPresentationEventIndex))
	{
		const float NextStartTime = PendingPresentationTimeline.Events[PendingPresentationEventIndex].StartTime;
		Delay = FMath::Max(Delay, NextStartTime - BatchStartTime);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PresentationEventTimerHandle,
			this,
			&UOrigamiBirdBoardWidget::HandlePresentationEventDelayFinished,
			Delay,
			false);
	}
}

void UOrigamiBirdBoardWidget::PlayPresentationEvent(const FOrigamiBirdPresentationEvent& Event)
{
	const float EventDuration = FMath::Max(Event.Duration, DefaultEventDuration);

	switch (Event.EventType)
	{
	case EOrigamiBirdPresentationEventType::Swap:
	case EOrigamiBirdPresentationEventType::Fall:
		for (const FOrigamiBirdTileTransition& Transition : Event.TileTransitions)
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
				EventDuration,
				EHoyoWidgetTweenEasing::EaseOut,
				2.0f);
		}
		break;

	case EOrigamiBirdPresentationEventType::MatchHighlight:
		for (const FOrigamiBirdTile& Tile : Event.AffectedTiles)
		{
			if (UOrigamiBirdTileVisualWidget* TileVisual = FindTileVisualById(Tile.TileId))
			{
				UHoyoWidgetTweenLibrary::TweenRenderScale(
					TileVisual,
					FVector2D(1.0f, 1.0f),
					FVector2D(1.12f, 1.12f),
					EventDuration,
					EHoyoWidgetTweenEasing::EaseOut,
					2.0f);
			}
		}
		break;

	case EOrigamiBirdPresentationEventType::Remove:
		for (const FOrigamiBirdTile& Tile : Event.AffectedTiles)
		{
			if (UOrigamiBirdTileVisualWidget* TileVisual = FindTileVisualById(Tile.TileId))
			{
				UHoyoWidgetTweenLibrary::TweenRenderScale(
					TileVisual,
					FVector2D(1.12f, 1.12f),
					FVector2D(0.2f, 0.2f),
					EventDuration,
					EHoyoWidgetTweenEasing::EaseIn,
					2.0f);
				UHoyoWidgetTweenLibrary::TweenRenderOpacity(
					TileVisual,
					1.0f,
					0.0f,
					EventDuration,
					EHoyoWidgetTweenEasing::EaseIn,
					2.0f);
				TileIdsPendingRemoval.AddUnique(Tile.TileId);
			}
		}
		break;

	case EOrigamiBirdPresentationEventType::Spawn:
		for (const FOrigamiBirdTile& Tile : Event.AffectedTiles)
		{
			if (UOrigamiBirdTileVisualWidget* TileVisual = CreateTileVisual(Tile))
			{
				AddTileVisualToGrid(TileVisual, Tile.BoardPosition);
				TileVisual->SetRenderScale(FVector2D(0.2f, 0.2f));
				TileVisual->SetRenderOpacity(0.0f);
				TileVisual->SetRenderTranslation(FVector2D(0.0f, -TileCellSize));
				UHoyoWidgetTweenLibrary::TweenRenderTranslation(
					TileVisual,
					FVector2D(0.0f, -TileCellSize),
					FVector2D::ZeroVector,
					EventDuration,
					EHoyoWidgetTweenEasing::EaseOut,
					2.0f);
				UHoyoWidgetTweenLibrary::TweenRenderScale(
					TileVisual,
					FVector2D(0.2f, 0.2f),
					FVector2D(1.0f, 1.0f),
					EventDuration,
					EHoyoWidgetTweenEasing::EaseOut,
					2.0f);
				UHoyoWidgetTweenLibrary::TweenRenderOpacity(
					TileVisual,
					0.0f,
					1.0f,
					EventDuration,
					EHoyoWidgetTweenEasing::EaseOut,
					2.0f);
			}
		}
		break;

	case EOrigamiBirdPresentationEventType::BoardSync:
		if (!Event.SnapshotAfterEvent.Tiles.IsEmpty())
		{
			ReconcileWithSnapshot(Event.SnapshotAfterEvent);
		}
		break;

	case EOrigamiBirdPresentationEventType::Score:
	default:
		break;
	}
}

void UOrigamiBirdBoardWidget::FinishPresentationTimeline()
{
	if (!PendingPresentationTimeline.FinalSnapshot.Tiles.IsEmpty())
	{
		ReconcileWithSnapshot(PendingPresentationTimeline.FinalSnapshot);
	}

	PendingPresentationTimeline = FOrigamiBirdPresentationTimeline();
	PendingPresentationEventIndex = 0;
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
