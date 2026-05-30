#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdMatchTypes.h"

// Compiles gameplay action facts into UI presentation timing.
// GameObject fills BoardChangeSteps/ResolveCycles; this type is the only place
// that knows how those facts become a PresentationTimeline.
struct HOYOGAS_API FOrigamiBirdActionPresentationBuilder
{
	static FOrigamiBirdPresentationTimeline BuildTimeline(
		const FOrigamiBirdActionResult& ActionResult,
		const FOrigamiBirdPresentationConfig& PresentationConfig);

	static void BuildTimelineIntoResult(
		FOrigamiBirdActionResult& ActionResult,
		const FOrigamiBirdPresentationConfig& PresentationConfig);
};
