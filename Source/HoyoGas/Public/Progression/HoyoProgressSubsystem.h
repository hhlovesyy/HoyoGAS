#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Progression/HoyoCheckTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TimerManager.h"
#include "HoyoProgressSubsystem.generated.h"

class UHoyoProgressSaveGame;
class UHoyoConditionEvaluator;
class UMyGameplayTagEventBusSubsystem;
struct FMyGameplayTagEvent;

DECLARE_MULTICAST_DELEGATE(FHoyoProgressChangedDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FHoyoProgressKeyChangedDelegate, FName);

UCLASS()
class HOYOGAS_API UHoyoProgressSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	int32 GetCounter(FGameplayTag CounterTag) const;
	void IncrementCounter(FGameplayTag CounterTag, int32 Delta = 1);
	bool GetValue(FGameplayTag ValueTag, FHoyoProgressValue& OutValue) const;
	void SetValue(FGameplayTag ValueTag, const FHoyoProgressValue& Value);
	void FlushToDisk(bool bForceSynchronous = false);

	bool IsAchievementUnlocked(FName AchievementId) const;
	void UnlockAchievement(FName AchievementId);

	bool Evaluate(const FHoyoCheckConditionSet& ConditionSet) const;
	bool EvaluateCondition(const FHoyoCheckCondition& Condition) const;
	void GetDependencyKeysForCondition(const FHoyoCheckCondition& Condition, TArray<FName>& OutDependencyKeys) const;

	FHoyoProgressChangedDelegate& OnProgressChanged();
	FHoyoProgressKeyChangedDelegate& OnProgressKeyChanged();

	static FName MakeValueDependencyKey(FGameplayTag ValueTag);
	static FName MakeAchievementDependencyKey(FName AchievementId);

private:
	void LoadProgress();
	void SaveProgressSync();
	void SaveProgressAsync();
	void HandleAsyncSaveComplete(const FString& SlotName, int32 UserIndex, bool bSuccess);
	void MarkDirty();
	void HandleAutoSaveTimer();
	void BindEventBus();
	void UnbindEventBus();
	void HandleMappedEvent(const FMyGameplayTagEvent& Event);
	void RegisterDefaultEvaluators();
	void RegisterEvaluator(UHoyoConditionEvaluator* Evaluator);

	UPROPERTY(Transient)
	TObjectPtr<UHoyoProgressSaveGame> SaveGame;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FHoyoProgressValue> Values;

	UPROPERTY(Transient)
	TSet<FName> UnlockedAchievementIds;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHoyoConditionEvaluator>> OwnedEvaluators;

	TWeakObjectPtr<UMyGameplayTagEventBusSubsystem> EventBus;
	TMap<FGameplayTag, FDelegateHandle> EventSubscriptionHandles;
	TMap<FGameplayTag, TObjectPtr<UHoyoConditionEvaluator>> EvaluatorsByCheckTag;
	FHoyoProgressChangedDelegate ProgressChangedDelegate;
	FHoyoProgressKeyChangedDelegate ProgressKeyChangedDelegate;
	FTimerHandle AutoSaveTimerHandle;
	bool bIsDirty = false;
	bool bAsyncSaveInFlight = false;
	bool bPendingSaveAfterAsync = false;
	bool bPendingForceSyncAfterAsync = false;
};
