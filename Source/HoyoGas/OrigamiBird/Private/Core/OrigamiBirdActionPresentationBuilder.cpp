#include "Core/OrigamiBirdActionPresentationBuilder.h"

#include "Core/OrigamiBirdResolvePresentationCompiler.h"

namespace
{
	EOrigamiBirdPresentationEventType ToPresentationEventType(EOrigamiBirdBoardChangeStepType StepType)
	{
		switch (StepType)
		{
		case EOrigamiBirdBoardChangeStepType::Swap:
			return EOrigamiBirdPresentationEventType::Swap;
		case EOrigamiBirdBoardChangeStepType::Remove:
			return EOrigamiBirdPresentationEventType::Remove;
		case EOrigamiBirdBoardChangeStepType::Fall:
			return EOrigamiBirdPresentationEventType::Fall;
		case EOrigamiBirdBoardChangeStepType::Spawn:
			return EOrigamiBirdPresentationEventType::Spawn;
		case EOrigamiBirdBoardChangeStepType::BoardSync:
			return EOrigamiBirdPresentationEventType::BoardSync;
		default:
			return EOrigamiBirdPresentationEventType::None;
		}
	}

	FOrigamiBirdPresentationEvent MakePresentationEvent(const FOrigamiBirdBoardChangeStep& Step)
	{
		FOrigamiBirdPresentationEvent Event;
		Event.EventType = ToPresentationEventType(Step.StepType);
		Event.TileTransitions = Step.TileTransitions;
		Event.AffectedTiles = Step.AffectedTiles;
		Event.AffectedPositions = Step.AffectedPositions;
		Event.RemovedTileCount = Step.AffectedPositions.Num();
		Event.SnapshotAfterEvent = Step.SnapshotAfterStep;
		return Event;
	}
}

FOrigamiBirdPresentationTimeline FOrigamiBirdActionPresentationBuilder::BuildTimeline(
	const FOrigamiBirdActionResult& ActionResult,
	const FOrigamiBirdPresentationConfig& PresentationConfig)
{
	FOrigamiBirdPresentationTimeline Timeline;

	for (const FOrigamiBirdBoardChangeStep& Step : ActionResult.BoardChangeSteps)
	{
		FOrigamiBirdPresentationEvent Event = MakePresentationEvent(Step);
		FOrigamiBirdResolvePresentationCompiler::AppendEvent(Timeline, MoveTemp(Event), &PresentationConfig);
	}

	FOrigamiBirdResolvePresentationCompiler::AppendResolveCycles(
		Timeline,
		ActionResult.ResolveCycles,
		&PresentationConfig);

	Timeline.FinalSnapshot = ActionResult.FinalSnapshot;
	return Timeline;
}

void FOrigamiBirdActionPresentationBuilder::BuildTimelineIntoResult(
	FOrigamiBirdActionResult& ActionResult,
	const FOrigamiBirdPresentationConfig& PresentationConfig)
{
	ActionResult.PresentationTimeline = BuildTimeline(ActionResult, PresentationConfig);
}
