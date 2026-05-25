#include "Stores/ProgressionUIStore.h"

#include "Components/ScoreProgressionComponent.h"
#include "Data/LevelProgressionRow.h"
#include "GameplayDemoStatics.h"
#include "HoyoGasPlayerState.h"

void UProgressionUIStore::BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState)
{
	Super::BindToPlayerContext(InPawn, InPlayerState);

	AHoyoGasPlayerState* HoyoGasPlayerState = Cast<AHoyoGasPlayerState>(InPlayerState);
	BindGameplaySources(HoyoGasPlayerState ? HoyoGasPlayerState->GetScoreProgressionComponent() : nullptr);
}

void UProgressionUIStore::UnbindFromPlayerContext()
{
	UnbindGameplaySources();
	Super::UnbindFromPlayerContext();
}

void UProgressionUIStore::BindGameplaySources(UScoreProgressionComponent* InScoreComponent)
{
	if (ScoreComponent.Get() == InScoreComponent)
	{
		RefreshFromScoreComponent();
		return;
	}

	UnbindGameplaySources();
	ScoreComponent = InScoreComponent;

	if (ScoreComponent.IsValid())
	{
		ScoreComponent->OnScoreChanged.AddDynamic(this, &UProgressionUIStore::HandleScoreChanged);
		ScoreComponent->OnLevelChanged.AddDynamic(this, &UProgressionUIStore::HandleLevelChanged);
	}

	RefreshFromScoreComponent();
}

void UProgressionUIStore::UnbindGameplaySources()
{
	if (ScoreComponent.IsValid())
	{
		ScoreComponent->OnScoreChanged.RemoveDynamic(this, &UProgressionUIStore::HandleScoreChanged);
		ScoreComponent->OnLevelChanged.RemoveDynamic(this, &UProgressionUIStore::HandleLevelChanged);
	}

	ScoreComponent.Reset();
	CurrentScore = 0;
	CurrentLevel = 1;
	ScoreToNextLevel = 0;
	ProgressToNextLevel01 = 0.0f;
	ConsumePendingLevelUp();
	BroadcastStoreChanged();
}

void UProgressionUIStore::RefreshFromScoreComponent()
{
	CurrentScore = ScoreComponent.IsValid() ? ScoreComponent->GetCurrentScore() : 0;
	CurrentLevel = ScoreComponent.IsValid() ? ScoreComponent->GetCurrentLevel() : 1;
	ScoreToNextLevel = ScoreComponent.IsValid() ? ScoreComponent->GetScoreToNextLevel() : 0;
	ProgressToNextLevel01 = ScoreComponent.IsValid() ? ScoreComponent->GetProgressToNextLevel01() : 0.0f;
	BroadcastStoreChanged();
}

int32 UProgressionUIStore::GetCurrentScore() const
{
	return CurrentScore;
}

int32 UProgressionUIStore::GetCurrentLevel() const
{
	return CurrentLevel;
}

int32 UProgressionUIStore::GetScoreToNextLevel() const
{
	return ScoreToNextLevel;
}

float UProgressionUIStore::GetProgressToNextLevel01() const
{
	return ProgressToNextLevel01;
}

bool UProgressionUIStore::HasPendingLevelUp() const
{
	return bHasPendingLevelUp;
}

FPendingLevelUpSnapshot UProgressionUIStore::GetPendingLevelUpSnapshot() const
{
	FPendingLevelUpSnapshot Snapshot;
	Snapshot.bHasPendingLevelUp = bHasPendingLevelUp;
	Snapshot.PendingLevel = PendingLevel;
	Snapshot.PendingLevelText = PendingLevelText;
	Snapshot.PendingRewardText = PendingRewardText;
	return Snapshot;
}

void UProgressionUIStore::ConsumePendingLevelUp()
{
	if (!bHasPendingLevelUp && PendingLevel == 0 && PendingRewardText.IsEmpty() && PendingLevelText.IsEmpty())
	{
		return;
	}

	bHasPendingLevelUp = false;
	PendingLevel = 0;
	PendingLevelText = FText::GetEmpty();
	PendingRewardText = FText::GetEmpty();
	BroadcastStoreChanged();
}

void UProgressionUIStore::HandleScoreChanged(int32 NewScore)
{
	RefreshFromScoreComponent();
}

void UProgressionUIStore::HandleLevelChanged(int32 OldLevel, int32 NewLevel)
{
	UpdatePendingLevelUp(NewLevel);
	RefreshFromScoreComponent();
}

void UProgressionUIStore::UpdatePendingLevelUp(int32 NewLevel)
{
	bHasPendingLevelUp = true;
	PendingLevel = NewLevel;
	PendingLevelText = FText::Format(INVTEXT("Reached Level {0}"), FText::AsNumber(NewLevel));
	PendingRewardText = INVTEXT("Progress milestone reached.");

	TArray<FLevelProgressionRow> LevelRows;
	UGameplayDemoStatics::GetAllLevelProgressionRows(this, LevelRows);

	for (const FLevelProgressionRow& Row : LevelRows)
	{
		if (Row.Level != NewLevel)
		{
			continue;
		}

		PendingLevelText = FText::Format(INVTEXT("Reached {0}"), Row.DisplayName);
		PendingRewardText = Row.RewardText;
		break;
	}
}
