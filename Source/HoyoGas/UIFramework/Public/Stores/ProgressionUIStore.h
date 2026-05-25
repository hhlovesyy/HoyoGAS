#pragma once

#include "CoreMinimal.h"
#include "Stores/MyUIStoreBase.h"
#include "Stores/MyUIStoreTypes.h"
#include "ProgressionUIStore.generated.h"

class UScoreProgressionComponent;

UCLASS(BlueprintType)
class HOYOGAS_API UProgressionUIStore : public UUIStoreBase
{
	GENERATED_BODY()

public:
	virtual void BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState) override;
	virtual void UnbindFromPlayerContext() override;

	void BindGameplaySources(UScoreProgressionComponent* InScoreComponent);
	void UnbindGameplaySources();
	void RefreshFromScoreComponent();

	int32 GetCurrentScore() const;
	int32 GetCurrentLevel() const;
	int32 GetScoreToNextLevel() const;
	float GetProgressToNextLevel01() const;
	bool HasPendingLevelUp() const;
	FPendingLevelUpSnapshot GetPendingLevelUpSnapshot() const;
	void ConsumePendingLevelUp();

private:
	UFUNCTION()
	void HandleScoreChanged(int32 NewScore);

	UFUNCTION()
	void HandleLevelChanged(int32 OldLevel, int32 NewLevel);

	void UpdatePendingLevelUp(int32 NewLevel);

	UPROPERTY(Transient)
	TWeakObjectPtr<UScoreProgressionComponent> ScoreComponent;

	UPROPERTY(Transient)
	int32 CurrentScore = 0;

	UPROPERTY(Transient)
	int32 CurrentLevel = 1;

	UPROPERTY(Transient)
	int32 ScoreToNextLevel = 0;

	UPROPERTY(Transient)
	float ProgressToNextLevel01 = 0.0f;

	UPROPERTY(Transient)
	bool bHasPendingLevelUp = false;

	UPROPERTY(Transient)
	int32 PendingLevel = 0;

	UPROPERTY(Transient)
	FText PendingLevelText;

	UPROPERTY(Transient)
	FText PendingRewardText;
};
