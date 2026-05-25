#include "Progression/HoyoAchievementSubsystem.h"

#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Progression/HoyoProgressionSettings.h"
#include "Progression/HoyoProgressionTags.h"
#include "Progression/HoyoProgressSubsystem.h"
#include "Subsystems/MyPlayerUISubsystem.h"

namespace
{
	constexpr int32 MaxAchievementQueueSteps = 1024;
}

void UHoyoAchievementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UHoyoProgressSubsystem::StaticClass());
	Super::Initialize(Collection);
	BindProgressSubsystem();
	RebuildAchievementIndex();
	EvaluateAchievements();
}

void UHoyoAchievementSubsystem::Deinitialize()
{
	UnbindProgressSubsystem();
	PendingAchievementIds.Reset();
	QueuedAchievementIds.Reset();
	AchievementIdsByDependencyKey.Reset();
	AchievementsById.Reset();
	Super::Deinitialize();
}

void UHoyoAchievementSubsystem::EvaluateAchievements()
{
	if (!ProgressSubsystem.IsValid())
	{
		return;
	}

	for (const TPair<FName, FHoyoAchievementDefinitionRow>& Pair : AchievementsById)
	{
		EnqueueAchievement(Pair.Key);
	}

	DrainAchievementQueue();
}

void UHoyoAchievementSubsystem::BindProgressSubsystem()
{
	UnbindProgressSubsystem();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		ProgressSubsystem = GameInstance->GetSubsystem<UHoyoProgressSubsystem>();
	}

	if (ProgressSubsystem.IsValid())
	{
		ProgressChangedHandle = ProgressSubsystem->OnProgressChanged().AddUObject(this, &UHoyoAchievementSubsystem::HandleProgressChanged);
		ProgressKeyChangedHandle = ProgressSubsystem->OnProgressKeyChanged().AddUObject(this, &UHoyoAchievementSubsystem::HandleProgressKeyChanged);
	}
}

void UHoyoAchievementSubsystem::UnbindProgressSubsystem()
{
	if (ProgressSubsystem.IsValid())
	{
		if (ProgressChangedHandle.IsValid())
		{
			ProgressSubsystem->OnProgressChanged().Remove(ProgressChangedHandle);
		}

		if (ProgressKeyChangedHandle.IsValid())
		{
			ProgressSubsystem->OnProgressKeyChanged().Remove(ProgressKeyChangedHandle);
		}
	}

	ProgressChangedHandle.Reset();
	ProgressKeyChangedHandle.Reset();
	ProgressSubsystem.Reset();
}

void UHoyoAchievementSubsystem::HandleProgressChanged()
{
	// UI and other broad listeners still use the no-arg progress signal. Achievement evaluation is driven by keyed changes.
}

void UHoyoAchievementSubsystem::HandleProgressKeyChanged(FName DependencyKey)
{
	UE_LOG(LogTemp, Display, TEXT("[Achievement] ProgressKeyChanged Key=%s"), *DependencyKey.ToString());
	EnqueueAchievementsForDependency(DependencyKey);
	DrainAchievementQueue();
}

void UHoyoAchievementSubsystem::GetAchievementDefinitions(TArray<FHoyoAchievementDefinitionRow>& OutRows) const
{
	OutRows.Reset();

	const UHoyoProgressionSettings* Settings = GetDefault<UHoyoProgressionSettings>();
	if (Settings && !Settings->AchievementDefinitionTable.IsNull())
	{
		if (UDataTable* AchievementTable = Settings->AchievementDefinitionTable.LoadSynchronous())
		{
			static const FString Context = TEXT("HoyoAchievementSubsystem::GetAchievementDefinitions");
			TArray<FHoyoAchievementDefinitionRow*> Rows;
			AchievementTable->GetAllRows(Context, Rows);
			for (const FHoyoAchievementDefinitionRow* Row : Rows)
			{
				if (Row)
				{
					OutRows.Add(*Row);
				}
			}
		}
	}
}

void UHoyoAchievementSubsystem::RebuildAchievementIndex()
{
	AchievementsById.Reset();
	AchievementIdsByDependencyKey.Reset();

	if (!ProgressSubsystem.IsValid())
	{
		return;
	}

	TArray<FHoyoAchievementDefinitionRow> Achievements;
	GetAchievementDefinitions(Achievements);

	for (const FHoyoAchievementDefinitionRow& Achievement : Achievements)
	{
		if (Achievement.AchievementId.IsNone())
		{
			continue;
		}

		AchievementsById.Add(Achievement.AchievementId, Achievement);

		TSet<FName> UniqueDependencyKeys;
		for (const FHoyoCheckCondition& Condition : Achievement.UnlockConditions.Conditions)
		{
			TArray<FName> DependencyKeys;
			ProgressSubsystem->GetDependencyKeysForCondition(Condition, DependencyKeys);
			for (const FName DependencyKey : DependencyKeys)
			{
				if (!DependencyKey.IsNone())
				{
					UniqueDependencyKeys.Add(DependencyKey);
				}
			}
		}

		for (const FName DependencyKey : UniqueDependencyKeys)
		{
			AchievementIdsByDependencyKey.FindOrAdd(DependencyKey).Add(Achievement.AchievementId);
			UE_LOG(LogTemp, Display, TEXT("[Achievement] DependencyIndex Key=%s Achievement=%s"),
				*DependencyKey.ToString(),
				*Achievement.AchievementId.ToString());
		}
	}

	UE_LOG(LogTemp, Display, TEXT("[Achievement] IndexReady Achievements=%d DependencyKeys=%d"),
		AchievementsById.Num(),
		AchievementIdsByDependencyKey.Num());
}

void UHoyoAchievementSubsystem::EnqueueAchievement(FName AchievementId)
{
	if (AchievementId.IsNone() || QueuedAchievementIds.Contains(AchievementId))
	{
		return;
	}

	if (!AchievementsById.Contains(AchievementId))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Achievement] Enqueue ignored because achievement is not indexed. Achievement=%s"), *AchievementId.ToString());
		return;
	}

	if (ProgressSubsystem.IsValid() && ProgressSubsystem->IsAchievementUnlocked(AchievementId))
	{
		UE_LOG(LogTemp, Display, TEXT("[Achievement] Enqueue skipped because achievement is already unlocked. Achievement=%s"), *AchievementId.ToString());
		return;
	}

	QueuedAchievementIds.Add(AchievementId);
	PendingAchievementIds.Add(AchievementId);
	UE_LOG(LogTemp, Display, TEXT("[Achievement] Enqueued Achievement=%s QueueSize=%d"),
		*AchievementId.ToString(),
		PendingAchievementIds.Num());
}

void UHoyoAchievementSubsystem::EnqueueAchievementsForDependency(FName DependencyKey)
{
	if (DependencyKey.IsNone())
	{
		return;
	}

	const TArray<FName>* AchievementIds = AchievementIdsByDependencyKey.Find(DependencyKey);
	if (!AchievementIds)
	{
		UE_LOG(LogTemp, Display, TEXT("[Achievement] No achievements depend on Key=%s"), *DependencyKey.ToString());
		return;
	}

	for (const FName AchievementId : *AchievementIds)
	{
		EnqueueAchievement(AchievementId);
	}
}

void UHoyoAchievementSubsystem::DrainAchievementQueue()
{
	if (bEvaluating || !ProgressSubsystem.IsValid())
	{
		return;
	}

	TGuardValue<bool> EvaluatingGuard(bEvaluating, true);

	int32 Steps = 0;
	while (!PendingAchievementIds.IsEmpty())
	{
		if (++Steps > MaxAchievementQueueSteps)
		{
			UE_LOG(LogTemp, Error, TEXT("[Achievement] Evaluation queue exceeded %d steps. Check achievement dependency cycles or invalid config."), MaxAchievementQueueSteps);
			PendingAchievementIds.Reset();
			QueuedAchievementIds.Reset();
			return;
		}

		const FName AchievementId = PendingAchievementIds[0];
		PendingAchievementIds.RemoveAt(0, 1, EAllowShrinking::No);
		QueuedAchievementIds.Remove(AchievementId);

		TryEvaluateAchievement(AchievementId);
	}
}

bool UHoyoAchievementSubsystem::TryEvaluateAchievement(FName AchievementId)
{
	if (!ProgressSubsystem.IsValid() || AchievementId.IsNone() || ProgressSubsystem->IsAchievementUnlocked(AchievementId))
	{
		return false;
	}

	const FHoyoAchievementDefinitionRow* Achievement = AchievementsById.Find(AchievementId);
	if (!Achievement)
	{
		return false;
	}

	if (!ProgressSubsystem->Evaluate(Achievement->UnlockConditions))
	{
		UE_LOG(LogTemp, Display, TEXT("[Achievement] Evaluate false Achievement=%s"), *AchievementId.ToString());
		return false;
	}

	UE_LOG(LogTemp, Display, TEXT("[Achievement] Unlock Achievement=%s"), *Achievement->AchievementId.ToString());
	ProgressSubsystem->UnlockAchievement(Achievement->AchievementId);
	ShowAchievementToast(*Achievement);
	return true;
}

void UHoyoAchievementSubsystem::ShowAchievementToast(const FHoyoAchievementDefinitionRow& Achievement) const
{
	const FText Message = Achievement.ToastMessage.IsEmpty()
		? FText::Format(NSLOCTEXT("HoyoAchievement", "DefaultAchievementToast", "Achievement unlocked: {0}"), Achievement.Title)
		: Achievement.ToastMessage;

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		for (ULocalPlayer* LocalPlayer : GameInstance->GetLocalPlayers())
		{
			if (!LocalPlayer)
			{
				continue;
			}

			if (UMyPlayerUISubsystem* PlayerUISubsystem = LocalPlayer->GetSubsystem<UMyPlayerUISubsystem>())
			{
				PlayerUISubsystem->ShowToast(Message, HoyoProgressionTags::UI_Request_Show_AchievementToast);
			}
		}
	}
}
