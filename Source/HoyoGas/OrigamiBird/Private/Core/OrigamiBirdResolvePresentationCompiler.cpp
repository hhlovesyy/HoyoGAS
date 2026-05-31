#include "Core/OrigamiBirdResolvePresentationCompiler.h"

namespace
{
	DEFINE_LOG_CATEGORY_STATIC(LogOrigamiBirdPresentationCompiler, Log, All);

	constexpr float InvalidConfigSafetyDuration = 0.01f;

	struct FResolvedPresentationTiming
	{
		float Duration = 0.0f;
		float OffsetFromPrevious = 0.0f;
		bool bStartWithPrevious = false;
	};

	FResolvedPresentationTiming ResolveTiming(
		EOrigamiBirdPresentationEventType EventType,
		const FOrigamiBirdPresentationConfig* PresentationConfig)
	{
		FResolvedPresentationTiming Timing;
		Timing.Duration = InvalidConfigSafetyDuration;

		if (!PresentationConfig)
		{
			UE_LOG(LogOrigamiBirdPresentationCompiler, Error, TEXT("PresentationConfig is missing for EventType=%d."), static_cast<int32>(EventType));
			return Timing;
		}

		for (const FOrigamiBirdPresentationTimingRule& Rule : PresentationConfig->TimingRules)
		{
			if (Rule.EventType != EventType)
			{
				continue;
			}

			if (Rule.Duration <= 0.0f)
			{
				UE_LOG(
					LogOrigamiBirdPresentationCompiler,
					Error,
					TEXT("PresentationConfig rule for EventType=%d has invalid Duration=%f."),
					static_cast<int32>(EventType),
					Rule.Duration);
				return Timing;
			}

			Timing.Duration = Rule.Duration;
			Timing.OffsetFromPrevious = Rule.OffsetFromPrevious;
			Timing.bStartWithPrevious = Rule.bStartWithPrevious;
			return Timing;
		}

		UE_LOG(LogOrigamiBirdPresentationCompiler, Error, TEXT("PresentationConfig has no TimingRule for EventType=%d."), static_cast<int32>(EventType));
		return Timing;
	}
}

void FOrigamiBirdResolvePresentationCompiler::AppendEvent(
	FOrigamiBirdPresentationTimeline& Timeline,
	FOrigamiBirdPresentationEvent Event,
	const FOrigamiBirdPresentationConfig* PresentationConfig)
{
	if (Event.EventType == EOrigamiBirdPresentationEventType::None)
	{
		return;
	}

	const FResolvedPresentationTiming Timing = ResolveTiming(Event.EventType, PresentationConfig);
	const float PreviousStartTime = Timeline.Events.IsEmpty() ? Timeline.TotalDuration : Timeline.Events.Last().StartTime;
	Event.StartTime = Timing.bStartWithPrevious
		? PreviousStartTime + Timing.OffsetFromPrevious
		: Timeline.TotalDuration + Timing.OffsetFromPrevious;
	Event.StartTime = FMath::Max(0.0f, Event.StartTime);

	if (Event.Duration <= 0.0f)
	{
		Event.Duration = Timing.Duration;
	}

	Timeline.Events.Add(Event);
	Timeline.TotalDuration = FMath::Max(Timeline.TotalDuration, Event.StartTime + Event.Duration);
}

void FOrigamiBirdResolvePresentationCompiler::AppendResolveCycles(
	FOrigamiBirdPresentationTimeline& Timeline,
	const TArray<FOrigamiBirdResolveCycle>& ResolveCycles,
	const FOrigamiBirdPresentationConfig* PresentationConfig)
{
	for (const FOrigamiBirdResolveCycle& Cycle : ResolveCycles)
	{
		FOrigamiBirdPresentationEvent MatchEvent;
		MatchEvent.EventType = EOrigamiBirdPresentationEventType::MatchHighlight;
		MatchEvent.AffectedPositions = Cycle.MatchPositions;
		MatchEvent.AffectedTiles = Cycle.MatchedTiles;
		MatchEvent.ComboIndex = Cycle.ComboIndex;
		MatchEvent.RemovedTileCount = Cycle.RemovedTileCount;
		MatchEvent.SnapshotAfterEvent = Cycle.SnapshotBeforeRemove;
		AppendEvent(Timeline, MatchEvent, PresentationConfig);

		FOrigamiBirdPresentationEvent RemoveEvent;
		RemoveEvent.EventType = EOrigamiBirdPresentationEventType::Remove;
		RemoveEvent.AffectedPositions = Cycle.RemovedPositions.IsEmpty() ? Cycle.MatchPositions : Cycle.RemovedPositions;
		RemoveEvent.AffectedTiles = Cycle.RemovedTiles;
		RemoveEvent.ComboIndex = Cycle.ComboIndex;
		RemoveEvent.RemovedTileCount = Cycle.RemovedTileCount;
		AppendEvent(Timeline, RemoveEvent, PresentationConfig);

		FOrigamiBirdPresentationEvent ScoreEvent;
		ScoreEvent.EventType = EOrigamiBirdPresentationEventType::Score;
		ScoreEvent.ScoreDelta = Cycle.ScoreDelta;
		ScoreEvent.ComboIndex = Cycle.ComboIndex;
		ScoreEvent.RemovedTileCount = Cycle.RemovedTileCount;
		ScoreEvent.SnapshotAfterEvent = Cycle.SnapshotAfterScore;
		AppendEvent(Timeline, ScoreEvent, PresentationConfig);

		if (!Cycle.FallTransitions.IsEmpty())
		{
			FOrigamiBirdPresentationEvent FallEvent;
			FallEvent.EventType = EOrigamiBirdPresentationEventType::Fall;
			FallEvent.TileTransitions = Cycle.FallTransitions;
			FallEvent.SnapshotAfterEvent = Cycle.SnapshotAfterCollapse;
			AppendEvent(Timeline, FallEvent, PresentationConfig);
		}

		if (!Cycle.SpawnedTiles.IsEmpty())
		{
			FOrigamiBirdPresentationEvent SpawnEvent;
			SpawnEvent.EventType = EOrigamiBirdPresentationEventType::Spawn;
			SpawnEvent.AffectedTiles = Cycle.SpawnedTiles;
			SpawnEvent.AffectedPositions = Cycle.SpawnedPositions;
			SpawnEvent.SnapshotAfterEvent = Cycle.SnapshotAfterCollapse;
			AppendEvent(Timeline, SpawnEvent, PresentationConfig);
		}
	}
}

FOrigamiBirdPresentationTimeline FOrigamiBirdResolvePresentationCompiler::BuildTimelineFromResolveCycles(
	const TArray<FOrigamiBirdResolveCycle>& ResolveCycles,
	const FOrigamiBirdBoardSnapshot& FinalSnapshot,
	const FOrigamiBirdPresentationConfig* PresentationConfig)
{
	FOrigamiBirdPresentationTimeline Timeline;
	Timeline.FinalSnapshot = FinalSnapshot;
	AppendResolveCycles(Timeline, ResolveCycles, PresentationConfig);
	return Timeline;
}
