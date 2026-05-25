#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScoreProgressionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChangedSignature, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelChangedSignature, int32, OldLevel, int32, NewLevel);

UCLASS(ClassGroup = (GameplayDemo), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class HOYOGAS_API UScoreProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScoreProgressionComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "GameplayDemo|Score")
	void AddScore(int32 Delta);

	UFUNCTION(BlueprintPure, Category = "GameplayDemo|Score")
	int32 GetCurrentScore() const;

	UFUNCTION(BlueprintPure, Category = "GameplayDemo|Score")
	int32 GetCurrentLevel() const;

	UFUNCTION(BlueprintPure, Category = "GameplayDemo|Score")
	int32 GetScoreToNextLevel() const;

	UFUNCTION(BlueprintPure, Category = "GameplayDemo|Score")
	float GetProgressToNextLevel01() const;

	UPROPERTY(BlueprintAssignable, Category = "GameplayDemo|Score")
	FOnScoreChangedSignature OnScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "GameplayDemo|Score")
	FOnLevelChangedSignature OnLevelChanged;

private:
	void RecalculateLevel(bool bBroadcastChange);

	UPROPERTY(VisibleAnywhere, Category = "GameplayDemo|Score")
	int32 CurrentScore = 0;

	UPROPERTY(VisibleAnywhere, Category = "GameplayDemo|Score")
	int32 CurrentLevel = 1;
};
