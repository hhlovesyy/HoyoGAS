#include "Cards/SurvivorCardLoadoutComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Cards/SurvivorCardBehavior.h"
#include "Cards/SurvivorCardDefinition.h"
#include "Cards/SurvivorCardRuntimeData.h"
#include "GAS/SurvivorAbilitySet.h"
#include "Core/SurvivorArenaLog.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GameplayEffect.h"

bool FSurvivorAppliedCardHandles::IsEmpty() const
{
	return ActiveEffectHandles.IsEmpty() && GrantedAbilitySpecHandles.IsEmpty() && SpawnedActors.IsEmpty();
}

void FSurvivorAppliedCardHandles::Reset()
{
	ActiveEffectHandles.Reset();
	GrantedAbilitySpecHandles.Reset();
	SpawnedActors.Reset();
}

USurvivorCardLoadoutComponent::USurvivorCardLoadoutComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void USurvivorCardLoadoutComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogSurvivorArena, Log, TEXT("CardLoadoutComponent initialized. Owner=%s MaxCardSlots=%d"), *GetNameSafe(GetOwner()), MaxCardSlots);
}

void USurvivorCardLoadoutComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (const FSurvivorCardRuntimeInstance& RuntimeInstance : EquippedCardRuntimeInstances)
	{
		if (!RuntimeInstance.CardDefinition)
		{
			continue;
		}

		const int32 StackCount = GetRuntimeInstanceCount(RuntimeInstance.CardDefinition);
		if (StackCount <= 0)
		{
			continue;
		}

		const FSurvivorCardBehaviorContext Context = BuildBehaviorContext(
			RuntimeInstance.CardDefinition,
			StackCount,
			RuntimeInstance.RuntimeInstanceId);
		for (USurvivorCardBehavior* Behavior : RuntimeInstance.CardDefinition->Behaviors)
		{
			if (Behavior && Behavior->bReceivesBehaviorTick)
			{
				Behavior->TickBehavior(Context, DeltaTime);
			}
		}
	}
}

bool USurvivorCardLoadoutComponent::EquipCard(USurvivorCardDefinition* CardDefinition)
{
	if (!CardDefinition)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("EquipCard failed because CardDefinition is null. Owner=%s"), *GetNameSafe(GetOwner()));
		return false;
	}

	FString ValidationError;
	if (!CardDefinition->ValidateDefinition(&ValidationError))
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("EquipCard failed because card definition is invalid. Owner=%s Card=%s Error=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(CardDefinition),
			*ValidationError);
		return false;
	}

	if (!HasFreeCardSlot())
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("EquipCard failed because card slots are full. Owner=%s Card=%s UsedSlots=%d MaxSlots=%d"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(CardDefinition),
			GetUsedCardSlots(),
			MaxCardSlots);
		return false;
	}

	const int32 OldStackCount = GetRuntimeInstanceCount(CardDefinition);
	if (OldStackCount > 0)
	{
		if (CardDefinition->bUnique)
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("EquipCard failed because card is unique and already equipped. Owner=%s Card=%s"),
				*GetNameSafe(GetOwner()),
				*GetNameSafe(CardDefinition));
			return false;
		}

		if (OldStackCount >= CardDefinition->MaxStack)
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("EquipCard failed because card reached MaxStack. Owner=%s Card=%s StackCount=%d MaxStack=%d"),
				*GetNameSafe(GetOwner()),
				*GetNameSafe(CardDefinition),
				OldStackCount,
				CardDefinition->MaxStack);
			return false;
		}
	}

	const int32 RuntimeInstanceId = NextRuntimeCardInstanceId++;
	FSurvivorCardRuntimeInstance& NewRuntimeInstance = EquippedCardRuntimeInstances.AddDefaulted_GetRef();
	NewRuntimeInstance.RuntimeInstanceId = RuntimeInstanceId;
	NewRuntimeInstance.CardDefinition = CardDefinition;

	FSurvivorAppliedCardHandles AppliedHandles;
	if (!ApplyCardStack(CardDefinition, OldStackCount + 1, AppliedHandles, RuntimeInstanceId))
	{
		EquippedCardRuntimeInstances.RemoveAt(EquippedCardRuntimeInstances.Num() - 1);
		return false;
	}

	NewRuntimeInstance.AppliedHandles = MoveTemp(AppliedHandles);

	RebuildEquippedCardEntries();
	RecalculateAggregatedCardTags();
	BroadcastCardStackChanged(CardDefinition, OldStackCount, OldStackCount + 1, GatherRuntimeInstanceIds(CardDefinition));
	RefreshBehaviorTickState();

	UE_LOG(LogSurvivorArena, Log, TEXT("EquipCard succeeded. Owner=%s Card=%s UsedSlots=%d/%d RuntimeInstanceId=%d"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(CardDefinition),
		GetUsedCardSlots(),
		MaxCardSlots,
		RuntimeInstanceId);

	EmitCardDebugMessage(FString::Printf(TEXT("Card Equipped: %s (%d/%d)"), *GetNameSafe(CardDefinition), GetUsedCardSlots(), MaxCardSlots), FColor::Green);
	PrintCardTagSummary();
	BroadcastLoadoutChanged();
	return true;
}

bool USurvivorCardLoadoutComponent::UnequipCard(USurvivorCardDefinition* CardDefinition)
{
	if (!CardDefinition)
	{
		return false;
	}

	const int32 RuntimeInstanceIndex = FindLastRuntimeInstanceIndex(CardDefinition);
	if (RuntimeInstanceIndex == INDEX_NONE)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("UnequipCard failed because card is not equipped. Owner=%s Card=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(CardDefinition));
		return false;
	}

	const int32 OldStackCount = GetRuntimeInstanceCount(CardDefinition);
	const FSurvivorCardRuntimeInstance RuntimeInstance = EquippedCardRuntimeInstances[RuntimeInstanceIndex];
	const FSurvivorCardBehaviorContext BehaviorContext = BuildBehaviorContext(CardDefinition, OldStackCount, RuntimeInstance.RuntimeInstanceId);
	for (USurvivorCardBehavior* Behavior : CardDefinition->Behaviors)
	{
		if (Behavior)
		{
			Behavior->OnCardUnequipped(BehaviorContext, RuntimeInstance.AppliedHandles);
		}
	}

	RemoveAppliedHandles(RuntimeInstance.AppliedHandles);
	EquippedCardRuntimeInstances.RemoveAt(RuntimeInstanceIndex);

	RebuildEquippedCardEntries();
	RecalculateAggregatedCardTags();
	BroadcastCardStackChanged(CardDefinition, OldStackCount, OldStackCount - 1, GatherRuntimeInstanceIds(CardDefinition));
	RefreshBehaviorTickState();

	UE_LOG(LogSurvivorArena, Log, TEXT("UnequipCard succeeded. Owner=%s Card=%s UsedSlots=%d/%d RuntimeInstanceId=%d"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(CardDefinition),
		GetUsedCardSlots(),
		MaxCardSlots,
		RuntimeInstance.RuntimeInstanceId);

	EmitCardDebugMessage(FString::Printf(TEXT("Card Unequipped: %s (%d/%d)"), *GetNameSafe(CardDefinition), GetUsedCardSlots(), MaxCardSlots), FColor::Yellow);
	PrintCardTagSummary();
	BroadcastLoadoutChanged();
	return true;
}

void USurvivorCardLoadoutComponent::ClearCards()
{
	for (int32 Index = EquippedCardRuntimeInstances.Num() - 1; Index >= 0; --Index)
	{
		const FSurvivorCardRuntimeInstance RuntimeInstance = EquippedCardRuntimeInstances[Index];
		if (RuntimeInstance.CardDefinition)
		{
			const int32 OldStackCount = GetRuntimeInstanceCount(RuntimeInstance.CardDefinition);
			const FSurvivorCardBehaviorContext Context = BuildBehaviorContext(RuntimeInstance.CardDefinition, OldStackCount, RuntimeInstance.RuntimeInstanceId);
			for (USurvivorCardBehavior* Behavior : RuntimeInstance.CardDefinition->Behaviors)
			{
				if (Behavior)
				{
					Behavior->OnCardUnequipped(Context, RuntimeInstance.AppliedHandles);
				}
			}
		}

		RemoveAppliedHandles(RuntimeInstance.AppliedHandles);
	}

	EquippedCardRuntimeInstances.Reset();
	EquippedCardEntries.Reset();
	AggregatedCardTags.Reset();
	RefreshBehaviorTickState();

	UE_LOG(LogSurvivorArena, Log, TEXT("ClearCards completed. Owner=%s"), *GetNameSafe(GetOwner()));
	EmitCardDebugMessage(TEXT("Cards Cleared"), FColor::Silver);
	BroadcastLoadoutChanged();
}

void USurvivorCardLoadoutComponent::NotifyRunStarted()
{
	for (const FSurvivorCardRuntimeInstance& RuntimeInstance : EquippedCardRuntimeInstances)
	{
		if (!RuntimeInstance.CardDefinition)
		{
			continue;
		}

		const int32 StackCount = GetRuntimeInstanceCount(RuntimeInstance.CardDefinition);
		if (StackCount <= 0)
		{
			continue;
		}

		const FSurvivorCardBehaviorContext Context = BuildBehaviorContext(
			RuntimeInstance.CardDefinition,
			StackCount,
			RuntimeInstance.RuntimeInstanceId);
		for (USurvivorCardBehavior* Behavior : RuntimeInstance.CardDefinition->Behaviors)
		{
			if (Behavior)
			{
				Behavior->OnRunStarted(Context);
			}
		}
	}
}

void USurvivorCardLoadoutComponent::NotifyEnemyKilled(AActor* KilledEnemyActor, int32 KillCountDelta)
{
	for (const FSurvivorCardRuntimeInstance& RuntimeInstance : EquippedCardRuntimeInstances)
	{
		if (!RuntimeInstance.CardDefinition)
		{
			continue;
		}

		const int32 StackCount = GetRuntimeInstanceCount(RuntimeInstance.CardDefinition);
		if (StackCount <= 0)
		{
			continue;
		}

		FSurvivorCardEnemyKillContext Context;
		Context.CardContext = BuildBehaviorContext(
			RuntimeInstance.CardDefinition,
			StackCount,
			RuntimeInstance.RuntimeInstanceId);
		Context.KilledEnemyActor = KilledEnemyActor;
		Context.KillCountDelta = KillCountDelta;

		for (USurvivorCardBehavior* Behavior : RuntimeInstance.CardDefinition->Behaviors)
		{
			if (Behavior)
			{
				Behavior->OnEnemyKilled(Context);
			}
		}
	}
}

int32 USurvivorCardLoadoutComponent::GetUsedCardSlots() const
{
	return EquippedCardRuntimeInstances.Num();
}

bool USurvivorCardLoadoutComponent::HasFreeCardSlot() const
{
	return GetUsedCardSlots() < MaxCardSlots;
}

const FGameplayTagContainer& USurvivorCardLoadoutComponent::GetAggregatedCardTags() const
{
	return AggregatedCardTags;
}

const TArray<FSurvivorEquippedCardEntry>& USurvivorCardLoadoutComponent::GetEquippedCardEntries() const
{
	return EquippedCardEntries;
}

const TArray<FSurvivorCardRuntimeInstance>& USurvivorCardLoadoutComponent::GetEquippedCardRuntimeInstances() const
{
	return EquippedCardRuntimeInstances;
}

const FSurvivorCardRuntimeInstance* USurvivorCardLoadoutComponent::FindCardRuntimeInstanceById(int32 RuntimeInstanceId) const
{
	for (const FSurvivorCardRuntimeInstance& RuntimeInstance : EquippedCardRuntimeInstances)
	{
		if (RuntimeInstance.RuntimeInstanceId == RuntimeInstanceId)
		{
			return &RuntimeInstance;
		}
	}

	return nullptr;
}

FSurvivorCardRuntimeState* USurvivorCardLoadoutComponent::FindMutableCardRuntimeState(int32 RuntimeInstanceId)
{
	for (FSurvivorCardRuntimeInstance& RuntimeInstance : EquippedCardRuntimeInstances)
	{
		if (RuntimeInstance.RuntimeInstanceId == RuntimeInstanceId)
		{
			return &RuntimeInstance.RuntimeState;
		}
	}

	return nullptr;
}

USurvivorCardRuntimeData* USurvivorCardLoadoutComponent::FindCardRuntimeData(int32 RuntimeInstanceId, FName StateId) const
{
	if (StateId.IsNone())
	{
		return nullptr;
	}

	const FSurvivorCardRuntimeInstance* RuntimeInstance = FindCardRuntimeInstanceById(RuntimeInstanceId);
	if (!RuntimeInstance)
	{
		return nullptr;
	}

	for (const FSurvivorCardRuntimeDataEntry& Entry : RuntimeInstance->RuntimeState.RuntimeDataEntries)
	{
		if (Entry.StateId == StateId)
		{
			return Entry.RuntimeData;
		}
	}

	return nullptr;
}

USurvivorCardRuntimeData* USurvivorCardLoadoutComponent::FindOrAddCardRuntimeData(int32 RuntimeInstanceId, FName StateId, TSubclassOf<USurvivorCardRuntimeData> RuntimeDataClass)
{
	if (StateId.IsNone() || !RuntimeDataClass)
	{
		return nullptr;
	}

	for (FSurvivorCardRuntimeInstance& RuntimeInstance : EquippedCardRuntimeInstances)
	{
		if (RuntimeInstance.RuntimeInstanceId != RuntimeInstanceId)
		{
			continue;
		}

		for (FSurvivorCardRuntimeDataEntry& Entry : RuntimeInstance.RuntimeState.RuntimeDataEntries)
		{
			if (Entry.StateId == StateId)
			{
				return Entry.RuntimeData;
			}
		}

		FSurvivorCardRuntimeDataEntry& NewEntry = RuntimeInstance.RuntimeState.RuntimeDataEntries.AddDefaulted_GetRef();
		NewEntry.StateId = StateId;
		NewEntry.RuntimeData = NewObject<USurvivorCardRuntimeData>(this, RuntimeDataClass);
		return NewEntry.RuntimeData;
	}

	return nullptr;
}

void USurvivorCardLoadoutComponent::RecalculateAggregatedCardTags()
{
	AggregatedCardTags.Reset();

	for (const FSurvivorCardRuntimeInstance& RuntimeInstance : EquippedCardRuntimeInstances)
	{
		if (RuntimeInstance.CardDefinition)
		{
			AggregatedCardTags.AppendTags(RuntimeInstance.CardDefinition->CardTags);
		}
	}
}

void USurvivorCardLoadoutComponent::PrintCardTagSummary() const
{
	const FString Summary = BuildCardTagSummaryString();
	UE_LOG(LogSurvivorArena, Log, TEXT("Card tag summary. Owner=%s UsedSlots=%d/%d Tags=%s"),
		*GetNameSafe(GetOwner()),
		GetUsedCardSlots(),
		MaxCardSlots,
		*Summary);

	EmitCardDebugMessage(FString::Printf(TEXT("Card Tags: %s"), *Summary), FColor::Cyan);
}

int32 USurvivorCardLoadoutComponent::GetMaxCardSlots() const
{
	return MaxCardSlots;
}

FText USurvivorCardLoadoutComponent::GetEquippedCardsSummaryText() const
{
	return FText::FromString(BuildEquippedCardsSummaryString());
}

FText USurvivorCardLoadoutComponent::GetAggregatedCardTagsSummaryText() const
{
	return FText::FromString(BuildCardTagSummaryString());
}

FOnSurvivorLoadoutChanged& USurvivorCardLoadoutComponent::OnLoadoutChanged()
{
	return LoadoutChangedEvent;
}

UAbilitySystemComponent* USurvivorCardLoadoutComponent::ResolveOwnerASC() const
{
	if (const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		return AbilitySystemInterface->GetAbilitySystemComponent();
	}

	return nullptr;
}

FSurvivorCardBehaviorContext USurvivorCardLoadoutComponent::BuildBehaviorContext(USurvivorCardDefinition* CardDefinition, int32 StackCount, int32 RuntimeCardInstanceId) const
{
	FSurvivorCardBehaviorContext Context;
	Context.OwnerActor = GetOwner();
	Context.Pawn = Cast<APawn>(GetOwner());
	Context.PlayerState = Cast<APlayerState>(GetOwner());
	if (!Context.Pawn && Context.PlayerState)
	{
		Context.Pawn = Context.PlayerState->GetPawn();
	}

	Context.AbilitySystemComponent = ResolveOwnerASC();
	Context.LoadoutComponent = const_cast<USurvivorCardLoadoutComponent*>(this);
	Context.CardDefinition = CardDefinition;
	Context.RuntimeCardInstanceId = RuntimeCardInstanceId;
	Context.StackCount = StackCount;
	return Context;
}

bool USurvivorCardLoadoutComponent::ApplyCardStack(USurvivorCardDefinition* CardDefinition, int32 NewStackCount, FSurvivorAppliedCardHandles& OutAppliedHandles, int32 RuntimeCardInstanceId)
{
	UAbilitySystemComponent* ASC = ResolveOwnerASC();
	if (!ASC)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("EquipCard failed because owner ASC is null. Owner=%s Card=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(CardDefinition));
		return false;
	}

	const UWorld* World = ASC->GetWorld();
	const AActor* OwnerActor = ASC->GetOwnerActor();
	const bool bCanRunInStandalone = World == nullptr || World->GetNetMode() == NM_Standalone;
	const bool bHasAuthority = OwnerActor && OwnerActor->HasAuthority();
	if (!bHasAuthority && !bCanRunInStandalone)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("EquipCard skipped on non-authority owner. Owner=%s Card=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(CardDefinition));
		return false;
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : CardDefinition->EffectsToApply)
	{
		if (!EffectClass)
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("Card %s contains a null GameplayEffect entry."), *GetNameSafe(CardDefinition));
			continue;
		}

		const UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
		if (!EffectCDO)
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("Card %s failed to resolve GameplayEffect CDO for %s."), *GetNameSafe(CardDefinition), *GetNameSafe(*EffectClass));
			continue;
		}

		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(CardDefinition);
		OutAppliedHandles.ActiveEffectHandles.Add(ASC->ApplyGameplayEffectToSelf(EffectCDO, 1.0f, EffectContext));
	}

	for (USurvivorAbilitySet* AbilitySet : CardDefinition->AbilitySetsToGrant)
	{
		if (!AbilitySet)
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("Card %s contains a null AbilitySet entry."), *GetNameSafe(CardDefinition));
			continue;
		}

		const FSurvivorGrantedAbilitySetHandles GrantedHandles = AbilitySet->GiveToAbilitySystemAndCollect(ASC, CardDefinition);
		OutAppliedHandles.GrantedAbilitySpecHandles.Append(GrantedHandles.AbilitySpecHandles);
		OutAppliedHandles.ActiveEffectHandles.Append(GrantedHandles.ActiveEffectHandles);
	}

	const FSurvivorCardBehaviorContext BehaviorContext = BuildBehaviorContext(CardDefinition, NewStackCount, RuntimeCardInstanceId);
	for (USurvivorCardBehavior* Behavior : CardDefinition->Behaviors)
	{
		if (!Behavior)
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("Card %s contains a null Behavior entry."), *GetNameSafe(CardDefinition));
			continue;
		}

		if (!Behavior->OnCardEquipped(BehaviorContext, OutAppliedHandles))
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("Card behavior equip failed. Owner=%s Card=%s Behavior=%s RuntimeInstanceId=%d"),
				*GetNameSafe(GetOwner()),
				*GetNameSafe(CardDefinition),
				*GetNameSafe(Behavior),
				RuntimeCardInstanceId);
			RemoveAppliedHandles(OutAppliedHandles);
			OutAppliedHandles.Reset();
			return false;
		}
	}

	return true;
}

void USurvivorCardLoadoutComponent::RemoveAppliedHandles(const FSurvivorAppliedCardHandles& AppliedHandles)
{
	UAbilitySystemComponent* ASC = ResolveOwnerASC();
	if (!ASC)
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& AbilityHandle : AppliedHandles.GrantedAbilitySpecHandles)
	{
		if (AbilityHandle.IsValid())
		{
			ASC->ClearAbility(AbilityHandle);
		}
	}

	for (const FActiveGameplayEffectHandle& EffectHandle : AppliedHandles.ActiveEffectHandles)
	{
		if (EffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(EffectHandle);
		}
	}

	for (const TWeakObjectPtr<AActor>& SpawnedActor : AppliedHandles.SpawnedActors)
	{
		if (SpawnedActor.IsValid())
		{
			SpawnedActor->Destroy();
		}
	}
}

void USurvivorCardLoadoutComponent::RefreshBehaviorTickState()
{
	bool bNeedsTick = false;
	for (const FSurvivorCardRuntimeInstance& RuntimeInstance : EquippedCardRuntimeInstances)
	{
		if (!RuntimeInstance.CardDefinition)
		{
			continue;
		}

		for (USurvivorCardBehavior* Behavior : RuntimeInstance.CardDefinition->Behaviors)
		{
			if (Behavior && Behavior->bReceivesBehaviorTick)
			{
				bNeedsTick = true;
				break;
			}
		}

		if (bNeedsTick)
		{
			break;
		}
	}

	SetComponentTickEnabled(bNeedsTick);
}

void USurvivorCardLoadoutComponent::EmitCardDebugMessage(const FString& Message, const FColor& Color) const
{
	if (!bEnableOnScreenCardDebug || !GEngine)
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 4.0f, Color, Message);
}

FString USurvivorCardLoadoutComponent::BuildCardTagSummaryString() const
{
	FString Summary;
	bool bFirst = true;

	for (const FGameplayTag& Tag : AggregatedCardTags)
	{
		if (!bFirst)
		{
			Summary += TEXT(", ");
		}

		Summary += Tag.ToString();
		bFirst = false;
	}

	return Summary.IsEmpty() ? TEXT("<None>") : Summary;
}

FString USurvivorCardLoadoutComponent::BuildEquippedCardsSummaryString() const
{
	FString Summary;
	bool bFirst = true;

	for (const FSurvivorEquippedCardEntry& Entry : EquippedCardEntries)
	{
		if (!Entry.CardDefinition || Entry.StackCount <= 0)
		{
			continue;
		}

		if (!bFirst)
		{
			Summary += TEXT(", ");
		}

		Summary += Entry.CardDefinition->CardId.IsNone() ? GetNameSafe(Entry.CardDefinition) : Entry.CardDefinition->CardId.ToString();
		if (Entry.StackCount > 1)
		{
			Summary += FString::Printf(TEXT(" x%d"), Entry.StackCount);
		}

		bFirst = false;
	}

	return Summary.IsEmpty() ? TEXT("<None>") : Summary;
}

void USurvivorCardLoadoutComponent::RebuildEquippedCardEntries()
{
	EquippedCardEntries.Reset();

	for (const FSurvivorCardRuntimeInstance& RuntimeInstance : EquippedCardRuntimeInstances)
	{
		if (!RuntimeInstance.CardDefinition)
		{
			continue;
		}

		FSurvivorEquippedCardEntry* ExistingEntry = FindCardEntry(RuntimeInstance.CardDefinition);
		if (!ExistingEntry)
		{
			FSurvivorEquippedCardEntry& NewEntry = EquippedCardEntries.AddDefaulted_GetRef();
			NewEntry.CardDefinition = RuntimeInstance.CardDefinition;
			NewEntry.StackCount = 1;
			NewEntry.RuntimeInstanceIds.Add(RuntimeInstance.RuntimeInstanceId);
			continue;
		}

		ExistingEntry->StackCount += 1;
		ExistingEntry->RuntimeInstanceIds.Add(RuntimeInstance.RuntimeInstanceId);
	}
}

TArray<int32> USurvivorCardLoadoutComponent::GatherRuntimeInstanceIds(USurvivorCardDefinition* CardDefinition) const
{
	TArray<int32> RuntimeInstanceIds;
	for (const FSurvivorCardRuntimeInstance& RuntimeInstance : EquippedCardRuntimeInstances)
	{
		if (RuntimeInstance.CardDefinition == CardDefinition)
		{
			RuntimeInstanceIds.Add(RuntimeInstance.RuntimeInstanceId);
		}
	}

	return RuntimeInstanceIds;
}

int32 USurvivorCardLoadoutComponent::GetRuntimeInstanceCount(USurvivorCardDefinition* CardDefinition) const
{
	int32 InstanceCount = 0;
	for (const FSurvivorCardRuntimeInstance& RuntimeInstance : EquippedCardRuntimeInstances)
	{
		if (RuntimeInstance.CardDefinition == CardDefinition)
		{
			++InstanceCount;
		}
	}

	return InstanceCount;
}

int32 USurvivorCardLoadoutComponent::FindLastRuntimeInstanceIndex(USurvivorCardDefinition* CardDefinition) const
{
	for (int32 Index = EquippedCardRuntimeInstances.Num() - 1; Index >= 0; --Index)
	{
		if (EquippedCardRuntimeInstances[Index].CardDefinition == CardDefinition)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

void USurvivorCardLoadoutComponent::BroadcastLoadoutChanged()
{
	LoadoutChangedEvent.Broadcast(this);
}

void USurvivorCardLoadoutComponent::BroadcastCardStackChanged(USurvivorCardDefinition* CardDefinition, int32 OldStackCount, int32 NewStackCount, const TArray<int32>& RuntimeInstanceIds)
{
	if (!CardDefinition)
	{
		return;
	}

	for (const int32 RuntimeInstanceId : RuntimeInstanceIds)
	{
		const FSurvivorCardBehaviorContext Context = BuildBehaviorContext(CardDefinition, NewStackCount, RuntimeInstanceId);
		for (USurvivorCardBehavior* Behavior : CardDefinition->Behaviors)
		{
			if (Behavior)
			{
				Behavior->OnCardStackChanged(Context, OldStackCount, NewStackCount);
			}
		}
	}
}

FSurvivorEquippedCardEntry* USurvivorCardLoadoutComponent::FindCardEntry(USurvivorCardDefinition* CardDefinition)
{
	for (FSurvivorEquippedCardEntry& Entry : EquippedCardEntries)
	{
		if (Entry.CardDefinition == CardDefinition)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const FSurvivorEquippedCardEntry* USurvivorCardLoadoutComponent::FindCardEntry(USurvivorCardDefinition* CardDefinition) const
{
	for (const FSurvivorEquippedCardEntry& Entry : EquippedCardEntries)
	{
		if (Entry.CardDefinition == CardDefinition)
		{
			return &Entry;
		}
	}

	return nullptr;
}
