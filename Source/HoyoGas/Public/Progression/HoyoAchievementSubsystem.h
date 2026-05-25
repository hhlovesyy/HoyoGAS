#pragma once

#include "CoreMinimal.h"
#include "Progression/HoyoAchievementDefinitionRow.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HoyoAchievementSubsystem.generated.h"

class UHoyoProgressSubsystem;

UCLASS()
class HOYOGAS_API UHoyoAchievementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void EvaluateAchievements();

private:
	void BindProgressSubsystem();
	void UnbindProgressSubsystem();
	void HandleProgressChanged();
	void HandleProgressKeyChanged(FName DependencyKey);
	void GetAchievementDefinitions(TArray<FHoyoAchievementDefinitionRow>& OutRows) const;
	void RebuildAchievementIndex();
	void EnqueueAchievement(FName AchievementId);
	void EnqueueAchievementsForDependency(FName DependencyKey);
	void DrainAchievementQueue();
	bool TryEvaluateAchievement(FName AchievementId);
	void ShowAchievementToast(const FHoyoAchievementDefinitionRow& Achievement) const;

	TWeakObjectPtr<UHoyoProgressSubsystem> ProgressSubsystem;
	FDelegateHandle ProgressChangedHandle;
	FDelegateHandle ProgressKeyChangedHandle;
	TMap<FName, FHoyoAchievementDefinitionRow> AchievementsById;
	TMap<FName, TArray<FName>> AchievementIdsByDependencyKey;
	TArray<FName> PendingAchievementIds;
	TSet<FName> QueuedAchievementIds;
	bool bEvaluating = false;
};
