#include "Components/ScoreProgressionComponent.h"

#include "GameplayDemoStatics.h"
#include "Data/LevelProgressionRow.h"

UScoreProgressionComponent::UScoreProgressionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UScoreProgressionComponent::BeginPlay()
{
	Super::BeginPlay();
	RecalculateLevel(false);
}

void UScoreProgressionComponent::AddScore(int32 Delta)
{
	if (Delta <= 0)
	{
		return;
	}

	CurrentScore += Delta;
	OnScoreChanged.Broadcast(CurrentScore);
	RecalculateLevel(true);
}

int32 UScoreProgressionComponent::GetCurrentScore() const
{
	return CurrentScore;
}

int32 UScoreProgressionComponent::GetCurrentLevel() const
{
	return CurrentLevel;
}

int32 UScoreProgressionComponent::GetScoreToNextLevel() const
{
	TArray<FLevelProgressionRow> Rows;
	UGameplayDemoStatics::GetAllLevelProgressionRows(this, Rows);

	for (const FLevelProgressionRow& Row : Rows)
	{
		if (Row.RequiredScore > CurrentScore)
		{
			return FMath::Max(0, Row.RequiredScore - CurrentScore);
		}
	}

	return 0;
}

float UScoreProgressionComponent::GetProgressToNextLevel01() const
{
	TArray<FLevelProgressionRow> Rows;
	UGameplayDemoStatics::GetAllLevelProgressionRows(this, Rows);

	int32 CurrentThreshold = 0;
	int32 NextThreshold = CurrentScore;

	for (int32 Index = 0; Index < Rows.Num(); ++Index)
	{
		const FLevelProgressionRow& Row = Rows[Index];
		if (Row.RequiredScore <= CurrentScore)
		{
			CurrentThreshold = Row.RequiredScore;
			continue;
		}

		NextThreshold = Row.RequiredScore;
		break;
	}

	if (NextThreshold <= CurrentThreshold)
	{
		return 1.0f;
	}

	return FMath::Clamp(static_cast<float>(CurrentScore - CurrentThreshold) / static_cast<float>(NextThreshold - CurrentThreshold), 0.0f, 1.0f);
}

void UScoreProgressionComponent::RecalculateLevel(bool bBroadcastChange)
{
	TArray<FLevelProgressionRow> Rows;
	UGameplayDemoStatics::GetAllLevelProgressionRows(this, Rows);

	int32 NewLevel = 1;
	for (const FLevelProgressionRow& Row : Rows)
	{
		if (CurrentScore >= Row.RequiredScore)
		{
			NewLevel = FMath::Max(NewLevel, Row.Level);
		}
	}

	if (NewLevel != CurrentLevel)
	{
		const int32 PreviousLevel = CurrentLevel;
		CurrentLevel = NewLevel;
		if (bBroadcastChange)
		{
			OnLevelChanged.Broadcast(PreviousLevel, CurrentLevel);
		}
	}
}
