#include "Progression/HoyoProgressSubsystem.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Progression/HoyoConditionEvaluator.h"
#include "Progression/HoyoProgressionSettings.h"
#include "Progression/HoyoProgressSaveGame.h"
#include "Subsystems/MyGameplayTagEventBusSubsystem.h"

namespace
{
	void FillSaveGameSnapshot(UHoyoProgressSaveGame& SaveGame, const TMap<FGameplayTag, FHoyoProgressValue>& Values, const TSet<FName>& UnlockedAchievementIds)
	{
		SaveGame.Counters.Reset();
		SaveGame.Values.Reset();

		for (const TPair<FGameplayTag, FHoyoProgressValue>& Pair : Values)
		{
			if (!Pair.Key.IsValid())
			{
				continue;
			}

			FHoyoProgressValueSaveEntry& Entry = SaveGame.Values.AddDefaulted_GetRef();
			Entry.ValueTag = Pair.Key;
			Entry.Value = Pair.Value;

			if (Pair.Value.ValueType == EHoyoProgressValueType::Int)
			{
				FHoyoProgressCounterSaveEntry& LegacyEntry = SaveGame.Counters.AddDefaulted_GetRef();
				LegacyEntry.CounterTag = Pair.Key;
				LegacyEntry.Value = Pair.Value.IntValue;
			}
		}

		SaveGame.UnlockedAchievementIds = UnlockedAchievementIds.Array();
	}
}

void UHoyoProgressSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UMyGameplayTagEventBusSubsystem::StaticClass());
	Super::Initialize(Collection);

	RegisterDefaultEvaluators();
	LoadProgress();
	BindEventBus();

	const UHoyoProgressionSettings* Settings = GetDefault<UHoyoProgressionSettings>();
	const float AutoSaveInterval = Settings ? Settings->AutoSaveIntervalSeconds : 5.0f;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AutoSaveTimerHandle, this, &UHoyoProgressSubsystem::HandleAutoSaveTimer, FMath::Max(0.1f, AutoSaveInterval), true);
	}

	UE_LOG(LogTemp, Display, TEXT("[Progression] ProgressSubsystem initialized. Values=%d EventMappings=%d Evaluators=%d"),
		Values.Num(),
		Settings ? Settings->EventCounterMappings.Num() : 0,
		EvaluatorsByCheckTag.Num());
}

void UHoyoProgressSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	}

	UnbindEventBus();
	FlushToDisk(true);
	Super::Deinitialize();
}

int32 UHoyoProgressSubsystem::GetCounter(FGameplayTag CounterTag) const
{
	FHoyoProgressValue Value;
	if (!GetValue(CounterTag, Value))
	{
		return 0;
	}

	if (Value.ValueType == EHoyoProgressValueType::Float)
	{
		return FMath::RoundToInt(Value.FloatValue);
	}

	if (Value.ValueType == EHoyoProgressValueType::Bool)
	{
		return Value.BoolValue ? 1 : 0;
	}

	return Value.IntValue;
}

void UHoyoProgressSubsystem::IncrementCounter(FGameplayTag CounterTag, int32 Delta)
{
	if (!CounterTag.IsValid() || Delta == 0)
	{
		return;
	}

	const int32 NewValue = FMath::Max(0, GetCounter(CounterTag) + Delta);
	SetValue(CounterTag, FHoyoProgressValue::FromInt(NewValue));
	UE_LOG(LogTemp, Display, TEXT("[Progression] CounterChanged Tag=%s Delta=%d Value=%d"),
		*CounterTag.ToString(),
		Delta,
		NewValue);
}

bool UHoyoProgressSubsystem::GetValue(FGameplayTag ValueTag, FHoyoProgressValue& OutValue) const
{
	if (!ValueTag.IsValid())
	{
		return false;
	}

	if (const FHoyoProgressValue* FoundValue = Values.Find(ValueTag))
	{
		OutValue = *FoundValue;
		return true;
	}

	return false;
}

void UHoyoProgressSubsystem::SetValue(FGameplayTag ValueTag, const FHoyoProgressValue& Value)
{
	if (!ValueTag.IsValid())
	{
		return;
	}

	Values.Add(ValueTag, Value);
	MarkDirty();
	ProgressChangedDelegate.Broadcast();
	ProgressKeyChangedDelegate.Broadcast(MakeValueDependencyKey(ValueTag));
}

void UHoyoProgressSubsystem::FlushToDisk(bool bForceSynchronous)
{
	if (!SaveGame)
	{
		return;
	}

	if (bAsyncSaveInFlight)
	{
		if (bIsDirty)
		{
			bPendingSaveAfterAsync = true;
			bPendingForceSyncAfterAsync = bPendingForceSyncAfterAsync || bForceSynchronous;
			UE_LOG(LogTemp, Display, TEXT("[Progression] Save deferred because an async save is already in flight. ForceSync=%s"),
				bForceSynchronous ? TEXT("true") : TEXT("false"));
		}
		return;
	}

	if (!bIsDirty)
	{
		return;
	}

	if (bForceSynchronous)
	{
		SaveProgressSync();
		return;
	}

	SaveProgressAsync();
}

bool UHoyoProgressSubsystem::IsAchievementUnlocked(FName AchievementId) const
{
	return !AchievementId.IsNone() && UnlockedAchievementIds.Contains(AchievementId);
}

void UHoyoProgressSubsystem::UnlockAchievement(FName AchievementId)
{
	if (AchievementId.IsNone() || UnlockedAchievementIds.Contains(AchievementId))
	{
		return;
	}

	UnlockedAchievementIds.Add(AchievementId);
	MarkDirty();
	ProgressChangedDelegate.Broadcast();
	ProgressKeyChangedDelegate.Broadcast(MakeAchievementDependencyKey(AchievementId));
}

bool UHoyoProgressSubsystem::Evaluate(const FHoyoCheckConditionSet& ConditionSet) const
{
	if (ConditionSet.Conditions.IsEmpty())
	{
		return true;
	}

	if (ConditionSet.bMatchAll)
	{
		for (const FHoyoCheckCondition& Condition : ConditionSet.Conditions)
		{
			if (!EvaluateCondition(Condition))
			{
				return false;
			}
		}
		return true;
	}

	for (const FHoyoCheckCondition& Condition : ConditionSet.Conditions)
	{
		if (EvaluateCondition(Condition))
		{
			return true;
		}
	}

	return false;
}

bool UHoyoProgressSubsystem::EvaluateCondition(const FHoyoCheckCondition& Condition) const
{
	if (const TObjectPtr<UHoyoConditionEvaluator>* Evaluator = EvaluatorsByCheckTag.Find(Condition.CheckTag))
	{
		return IsValid(*Evaluator) && (*Evaluator)->Evaluate(Condition, *this);
	}

	UE_LOG(LogTemp, Warning, TEXT("[Progression] No condition evaluator registered for CheckTag=%s"), *Condition.CheckTag.ToString());
	return false;
}

void UHoyoProgressSubsystem::GetDependencyKeysForCondition(const FHoyoCheckCondition& Condition, TArray<FName>& OutDependencyKeys) const
{
	if (const TObjectPtr<UHoyoConditionEvaluator>* Evaluator = EvaluatorsByCheckTag.Find(Condition.CheckTag))
	{
		if (IsValid(*Evaluator))
		{
			(*Evaluator)->GetDependencyKeys(Condition, OutDependencyKeys);
		}
	}
}

FHoyoProgressChangedDelegate& UHoyoProgressSubsystem::OnProgressChanged()
{
	return ProgressChangedDelegate;
}

FHoyoProgressKeyChangedDelegate& UHoyoProgressSubsystem::OnProgressKeyChanged()
{
	return ProgressKeyChangedDelegate;
}

FName UHoyoProgressSubsystem::MakeValueDependencyKey(FGameplayTag ValueTag)
{
	return ValueTag.IsValid()
		? FName(*FString::Printf(TEXT("Value:%s"), *ValueTag.ToString()))
		: NAME_None;
}

FName UHoyoProgressSubsystem::MakeAchievementDependencyKey(FName AchievementId)
{
	return !AchievementId.IsNone()
		? FName(*FString::Printf(TEXT("Achievement:%s"), *AchievementId.ToString()))
		: NAME_None;
}

void UHoyoProgressSubsystem::LoadProgress()
{
	const UHoyoProgressionSettings* Settings = GetDefault<UHoyoProgressionSettings>();
	const FString SlotName = Settings ? Settings->SaveSlotName : TEXT("HoyoProgress");
	const int32 UserIndex = Settings ? Settings->SaveUserIndex : 0;

	SaveGame = Cast<UHoyoProgressSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	if (!SaveGame)
	{
		SaveGame = Cast<UHoyoProgressSaveGame>(UGameplayStatics::CreateSaveGameObject(UHoyoProgressSaveGame::StaticClass()));
	}

	Values.Reset();
	UnlockedAchievementIds.Reset();
	bIsDirty = false;

	if (!SaveGame)
	{
		return;
	}

	for (const FHoyoProgressValueSaveEntry& Entry : SaveGame->Values)
	{
		if (Entry.ValueTag.IsValid())
		{
			Values.Add(Entry.ValueTag, Entry.Value);
		}
	}

	for (const FHoyoProgressCounterSaveEntry& Entry : SaveGame->Counters)
	{
		if (Entry.CounterTag.IsValid() && !Values.Contains(Entry.CounterTag))
		{
			Values.Add(Entry.CounterTag, FHoyoProgressValue::FromInt(Entry.Value));
		}
	}

	for (const FName AchievementId : SaveGame->UnlockedAchievementIds)
	{
		if (!AchievementId.IsNone())
		{
			UnlockedAchievementIds.Add(AchievementId);
		}
	}
}

void UHoyoProgressSubsystem::SaveProgressSync()
{
	if (!SaveGame)
	{
		return;
	}

	if (bAsyncSaveInFlight)
	{
		bPendingSaveAfterAsync = true;
		bPendingForceSyncAfterAsync = true;
		UE_LOG(LogTemp, Display, TEXT("[Progression] Sync save deferred because an async save is already in flight."));
		return;
	}

	FillSaveGameSnapshot(*SaveGame, Values, UnlockedAchievementIds);

	const UHoyoProgressionSettings* Settings = GetDefault<UHoyoProgressionSettings>();
	const FString SlotName = Settings ? Settings->SaveSlotName : TEXT("HoyoProgress");
	const int32 UserIndex = Settings ? Settings->SaveUserIndex : 0;
	UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);
	bIsDirty = false;
	bPendingSaveAfterAsync = false;
	bPendingForceSyncAfterAsync = false;
}

void UHoyoProgressSubsystem::SaveProgressAsync()
{
	if (bAsyncSaveInFlight)
	{
		bPendingSaveAfterAsync = true;
		return;
	}

	UHoyoProgressSaveGame* Snapshot = Cast<UHoyoProgressSaveGame>(UGameplayStatics::CreateSaveGameObject(UHoyoProgressSaveGame::StaticClass()));
	if (!Snapshot)
	{
		SaveProgressSync();
		return;
	}

	FillSaveGameSnapshot(*Snapshot, Values, UnlockedAchievementIds);

	const UHoyoProgressionSettings* Settings = GetDefault<UHoyoProgressionSettings>();
	const FString SlotName = Settings ? Settings->SaveSlotName : TEXT("HoyoProgress");
	const int32 UserIndex = Settings ? Settings->SaveUserIndex : 0;

	bAsyncSaveInFlight = true;
	bIsDirty = false;
	bPendingSaveAfterAsync = false;
	FAsyncSaveGameToSlotDelegate SaveDelegate;
	SaveDelegate.BindUObject(this, &UHoyoProgressSubsystem::HandleAsyncSaveComplete);
	UGameplayStatics::AsyncSaveGameToSlot(Snapshot, SlotName, UserIndex, SaveDelegate);
}

void UHoyoProgressSubsystem::HandleAsyncSaveComplete(const FString& SlotName, int32 UserIndex, bool bSuccess)
{
	bAsyncSaveInFlight = false;
	UE_LOG(LogTemp, Display, TEXT("[Progression] AsyncSave complete Slot=%s UserIndex=%d Success=%s DirtyAfterSave=%s"),
		*SlotName,
		UserIndex,
		bSuccess ? TEXT("true") : TEXT("false"),
		bIsDirty ? TEXT("true") : TEXT("false"));

	if (!bPendingSaveAfterAsync && !bIsDirty)
	{
		return;
	}

	const bool bForceSync = bPendingForceSyncAfterAsync;
	bPendingSaveAfterAsync = false;
	bPendingForceSyncAfterAsync = false;

	if (bForceSync)
	{
		SaveProgressSync();
	}
	else
	{
		SaveProgressAsync();
	}
}

void UHoyoProgressSubsystem::MarkDirty()
{
	bIsDirty = true;
}

void UHoyoProgressSubsystem::HandleAutoSaveTimer()
{
	FlushToDisk(false);
}

void UHoyoProgressSubsystem::BindEventBus()
{
	UnbindEventBus();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		EventBus = GameInstance->GetSubsystem<UMyGameplayTagEventBusSubsystem>();
	}

	if (!EventBus.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Progression] Failed to bind event bus listeners because EventBus is unavailable."));
		return;
	}

	const UHoyoProgressionSettings* Settings = GetDefault<UHoyoProgressionSettings>();
	if (!Settings)
	{
		return;
	}

	if (Settings->EventCounterMappings.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Progression] No event counter mappings configured. Configure UHoyoProgressionSettings.EventCounterMappings in DefaultGame.ini or Project Settings."));
		return;
	}

	for (const FHoyoEventCounterMapping& Mapping : Settings->EventCounterMappings)
	{
		if (!Mapping.EventTag.IsValid() || !Mapping.CounterTag.IsValid())
		{
			continue;
		}

		const FDelegateHandle Handle = EventBus->Subscribe(
			Mapping.EventTag,
			FMyGameplayTagEventDelegate::FDelegate::CreateUObject(this, &UHoyoProgressSubsystem::HandleMappedEvent));
		EventSubscriptionHandles.Add(Mapping.EventTag, Handle);

		UE_LOG(LogTemp, Display, TEXT("[Progression] Bound event mapping EventTag=%s CounterTag=%s HandleValid=%s"),
			*Mapping.EventTag.ToString(),
			*Mapping.CounterTag.ToString(),
			Handle.IsValid() ? TEXT("true") : TEXT("false"));
	}
}

void UHoyoProgressSubsystem::UnbindEventBus()
{
	if (EventBus.IsValid())
	{
		for (const TPair<FGameplayTag, FDelegateHandle>& Pair : EventSubscriptionHandles)
		{
			if (Pair.Key.IsValid() && Pair.Value.IsValid())
			{
				EventBus->Unsubscribe(Pair.Key, Pair.Value);
			}
		}
	}

	EventSubscriptionHandles.Reset();
	EventBus.Reset();
}

void UHoyoProgressSubsystem::HandleMappedEvent(const FMyGameplayTagEvent& Event)
{
	const UHoyoProgressionSettings* Settings = GetDefault<UHoyoProgressionSettings>();
	if (!Settings)
	{
		return;
	}

	const FHoyoEventCounterMapping* FoundMapping = Settings->EventCounterMappings.FindByPredicate(
		[&Event](const FHoyoEventCounterMapping& Mapping)
		{
			return Mapping.EventTag == Event.EventTag;
		});

	if (!FoundMapping || !FoundMapping->CounterTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Progression] Mapped event received but no counter mapping exists. EventTag=%s"), *Event.EventTag.ToString());
		return;
	}

	IncrementCounter(FoundMapping->CounterTag, 1);
}

void UHoyoProgressSubsystem::RegisterDefaultEvaluators()
{
	RegisterEvaluator(NewObject<UHoyoCounterConditionEvaluator>(this));
	RegisterEvaluator(NewObject<UHoyoAchievementUnlockedConditionEvaluator>(this));
	const UHoyoProgressionSettings* Settings = GetDefault<UHoyoProgressionSettings>();
	if (!Settings)
	{
		return;
	}

	for (TSubclassOf<UHoyoConditionEvaluator> EvaluatorClass : Settings->ConditionEvaluatorClasses)
	{
		if (!EvaluatorClass)
		{
			continue;
		}

		RegisterEvaluator(NewObject<UHoyoConditionEvaluator>(this, EvaluatorClass));
	}
}

void UHoyoProgressSubsystem::RegisterEvaluator(UHoyoConditionEvaluator* Evaluator)
{
	if (!IsValid(Evaluator))
	{
		return;
	}

	const FGameplayTag EvaluatorTag = Evaluator->GetEvaluatorTag();
	if (!EvaluatorTag.IsValid())
	{
		return;
	}

	OwnedEvaluators.Add(Evaluator);
	EvaluatorsByCheckTag.Add(EvaluatorTag, Evaluator);
}
