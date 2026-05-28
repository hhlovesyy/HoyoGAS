#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdMatchTypes.h"

// Converts gameplay resolution facts into presentation events.
// UI only consumes FOrigamiBirdPresentationTimeline.
struct HOYOGAS_API FOrigamiBirdResolvePresentationCompiler
{
	static void AppendEvent(
		FOrigamiBirdPresentationTimeline& Timeline,
		FOrigamiBirdPresentationEvent Event,
		const FOrigamiBirdPresentationConfig* PresentationConfig = nullptr);

	static void AppendResolveCycles(
		FOrigamiBirdPresentationTimeline& Timeline,
		const TArray<FOrigamiBirdResolveCycle>& ResolveCycles,
		const FOrigamiBirdPresentationConfig* PresentationConfig = nullptr);

	static FOrigamiBirdPresentationTimeline BuildTimelineFromResolveCycles(
		const TArray<FOrigamiBirdResolveCycle>& ResolveCycles,
		const FOrigamiBirdBoardSnapshot& FinalSnapshot,
		const FOrigamiBirdPresentationConfig* PresentationConfig = nullptr);
};
