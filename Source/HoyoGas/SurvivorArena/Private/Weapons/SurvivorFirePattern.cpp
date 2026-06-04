#include "Weapons/SurvivorFirePattern.h"

void USurvivorFirePattern::GenerateDirections_Implementation(const FSurvivorFirePatternContext& Context, TArray<FVector>& OutDirections) const
{
}

void USurvivorForwardFirePattern::GenerateDirections_Implementation(const FSurvivorFirePatternContext& Context, TArray<FVector>& OutDirections) const
{
	FVector Direction = Context.ForwardDirection;
	Direction.Z = 0.0f;

	if (!Direction.Normalize())
	{
		Direction = FVector::ForwardVector;
	}

	OutDirections.Add(Direction);
}

void USurvivorRadialFirePattern::GenerateDirections_Implementation(const FSurvivorFirePatternContext& Context, TArray<FVector>& OutDirections) const
{
	const int32 SafeDirectionCount = FMath::Max(1, DirectionCount);

	float BaseAngleRadians = 0.0f;
	if (RadialAlignment == ESurvivorRadialAlignment::CharacterForward)
	{
		FVector BaseDirection = Context.ForwardDirection;
		BaseDirection.Z = 0.0f;
		if (!BaseDirection.Normalize())
		{
			BaseDirection = FVector::ForwardVector;
		}

		BaseAngleRadians = FMath::Atan2(BaseDirection.Y, BaseDirection.X);
	}

	const float AngleStepRadians = (2.0f * PI) / static_cast<float>(SafeDirectionCount);

	OutDirections.Reserve(OutDirections.Num() + SafeDirectionCount);

	for (int32 Index = 0; Index < SafeDirectionCount; ++Index)
	{
		const float AngleRadians = BaseAngleRadians + (AngleStepRadians * static_cast<float>(Index));
		FVector Direction(FMath::Cos(AngleRadians), FMath::Sin(AngleRadians), 0.0f);
		Direction.Normalize();
		OutDirections.Add(Direction);
	}
}
