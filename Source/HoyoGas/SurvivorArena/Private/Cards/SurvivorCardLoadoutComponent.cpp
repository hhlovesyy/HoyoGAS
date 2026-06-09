#include "Cards/SurvivorCardLoadoutComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Cards/SurvivorCardBehavior.h"
#include "Cards/SurvivorCardDefinition.h"
#include "Core/SurvivorArenaLog.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GameplayEffect.h"
#include "Weapons/SurvivorWeaponDefinition.h"
#include "Weapons/SurvivorWeaponManagerComponent.h"

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

	for (const FSurvivorEquippedCardEntry& Entry : EquippedCardEntries)
	{
		if (!Entry.CardDefinition || Entry.StackCount <= 0)
		{
			continue;
		}

		const FSurvivorCardBehaviorContext Context = BuildBehaviorContext(Entry.CardDefinition, Entry.StackCount);
		for (USurvivorCardBehavior* Behavior : Entry.CardDefinition->Behaviors)
		{
			if (Behavior && Behavior->bReceivesBehaviorTick)
			{
				Behavior->TickBehavior(Context, DeltaTime);
			}
		}
	}
}

bool USurvivorCardLoadoutComponent::GrantStartingAbilitySet(USurvivorAbilitySet* AbilitySet, UObject* SourceObject)
{
	if (!AbilitySet)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("GrantStartingAbilitySet failed because AbilitySet is null. Owner=%s"), *GetNameSafe(GetOwner()));
		return false;
	}

	UAbilitySystemComponent* ASC = ResolveOwnerASC();
	if (!ASC)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("GrantStartingAbilitySet failed because owner ASC is null. Owner=%s AbilitySet=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(AbilitySet));
		return false;
	}

	const FSurvivorGrantedAbilitySetHandles GrantedHandles = AbilitySet->GiveToAbilitySystemAndCollect(ASC, SourceObject);
	GrantedStartingAbilitySetHandles.Add(GrantedHandles);

	UE_LOG(LogSurvivorArena, Log, TEXT("Granted starting ability set. Owner=%s AbilitySet=%s Count=%d"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(AbilitySet),
		GrantedStartingAbilitySetHandles.Num());

	BroadcastLoadoutChanged();
	return true;
}

bool USurvivorCardLoadoutComponent::GrantStartingWeapon(USurvivorWeaponDefinition* WeaponDefinition, USurvivorWeaponManagerComponent* WeaponManager)
{
	if (!WeaponDefinition || !WeaponManager)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("GrantStartingWeapon failed because WeaponDefinition or WeaponManager is null. Owner=%s Weapon=%s WeaponManager=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(WeaponDefinition),
			*GetNameSafe(WeaponManager));
		return false;
	}

	FSurvivorGrantedWeaponHandles GrantedHandles;
	if (!WeaponManager->GrantWeaponAndCollect(WeaponDefinition, GrantedHandles))
	{
		return false;
	}

	GrantedStartingWeapons.Add(GrantedHandles);

	UE_LOG(LogSurvivorArena, Log, TEXT("Granted starting weapon. Owner=%s Weapon=%s Count=%d"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(WeaponDefinition),
		GrantedStartingWeapons.Num());

	BroadcastLoadoutChanged();
	return true;
}

void USurvivorCardLoadoutComponent::ClearStartingLoadout(USurvivorWeaponManagerComponent* WeaponManager)
{
	if (UAbilitySystemComponent* ASC = ResolveOwnerASC())
	{
		for (FSurvivorGrantedAbilitySetHandles& GrantedHandles : GrantedStartingAbilitySetHandles)
		{
			GrantedHandles.TakeFromAbilitySystem(ASC);
		}
	}

	GrantedStartingAbilitySetHandles.Reset();

	if (WeaponManager)
	{
		for (const FSurvivorGrantedWeaponHandles& GrantedHandles : GrantedStartingWeapons)
		{
			WeaponManager->RemoveWeaponByHandles(GrantedHandles);
		}
	}

	GrantedStartingWeapons.Reset();
	BroadcastLoadoutChanged();
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

	FSurvivorEquippedCardEntry* ExistingEntry = FindCardEntry(CardDefinition);
	const int32 OldStackCount = ExistingEntry ? ExistingEntry->StackCount : 0;
	if (ExistingEntry)
	{
		if (CardDefinition->bUnique)
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("EquipCard failed because card is unique and already equipped. Owner=%s Card=%s"),
				*GetNameSafe(GetOwner()),
				*GetNameSafe(CardDefinition));
			return false;
		}

		if (ExistingEntry->StackCount >= CardDefinition->MaxStack)
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("EquipCard failed because card reached MaxStack. Owner=%s Card=%s StackCount=%d MaxStack=%d"),
				*GetNameSafe(GetOwner()),
				*GetNameSafe(CardDefinition),
				ExistingEntry->StackCount,
				CardDefinition->MaxStack);
			return false;
		}
	}

	FSurvivorAppliedCardHandles AppliedHandles;
	if (!ApplyCardStack(CardDefinition, AppliedHandles))
	{
		return false;
	}

	if (!ExistingEntry)
	{
		FSurvivorEquippedCardEntry NewEntry;
		NewEntry.CardDefinition = CardDefinition;
		NewEntry.StackCount = 1;
		NewEntry.AppliedStacks.Add(MoveTemp(AppliedHandles));
		EquippedCardEntries.Add(MoveTemp(NewEntry));
	}
	else
	{
		ExistingEntry->StackCount += 1;
		ExistingEntry->AppliedStacks.Add(MoveTemp(AppliedHandles));
	}

	RecalculateAggregatedCardTags();
	BroadcastCardStackChanged(CardDefinition, OldStackCount, OldStackCount + 1);
	RefreshBehaviorTickState();

	UE_LOG(LogSurvivorArena, Log, TEXT("EquipCard succeeded. Owner=%s Card=%s UsedSlots=%d/%d"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(CardDefinition),
		GetUsedCardSlots(),
		MaxCardSlots);

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

	for (int32 Index = 0; Index < EquippedCardEntries.Num(); ++Index)
	{
		FSurvivorEquippedCardEntry& Entry = EquippedCardEntries[Index];
		if (Entry.CardDefinition != CardDefinition)
		{
			continue;
		}

		if (Entry.AppliedStacks.Num() > 0)
		{
			const FSurvivorAppliedCardHandles AppliedHandles = Entry.AppliedStacks.Pop();
			const FSurvivorCardBehaviorContext BehaviorContext = BuildBehaviorContext(CardDefinition, Entry.StackCount);
			for (USurvivorCardBehavior* Behavior : CardDefinition->Behaviors)
			{
				if (Behavior)
				{
					Behavior->OnCardUnequipped(BehaviorContext, AppliedHandles);
				}
			}

			RemoveAppliedHandles(AppliedHandles);
		}

		Entry.StackCount = FMath::Max(0, Entry.StackCount - 1);
		const int32 NewStackCount = Entry.StackCount;
		if (Entry.StackCount == 0)
		{
			EquippedCardEntries.RemoveAt(Index);
		}

		RecalculateAggregatedCardTags();
		BroadcastCardStackChanged(CardDefinition, NewStackCount + 1, NewStackCount);
		RefreshBehaviorTickState();

		UE_LOG(LogSurvivorArena, Log, TEXT("UnequipCard succeeded. Owner=%s Card=%s UsedSlots=%d/%d"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(CardDefinition),
			GetUsedCardSlots(),
			MaxCardSlots);

		EmitCardDebugMessage(FString::Printf(TEXT("Card Unequipped: %s (%d/%d)"), *GetNameSafe(CardDefinition), GetUsedCardSlots(), MaxCardSlots), FColor::Yellow);
		PrintCardTagSummary();
		BroadcastLoadoutChanged();
		return true;
	}

	UE_LOG(LogSurvivorArena, Warning, TEXT("UnequipCard failed because card is not equipped. Owner=%s Card=%s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(CardDefinition));
	return false;
}

void USurvivorCardLoadoutComponent::ClearCards()
{
	for (const FSurvivorEquippedCardEntry& Entry : EquippedCardEntries)
	{
		for (const FSurvivorAppliedCardHandles& AppliedHandles : Entry.AppliedStacks)
		{
			RemoveAppliedHandles(AppliedHandles);
		}
	}

	EquippedCardEntries.Reset();
	AggregatedCardTags.Reset();
	RefreshBehaviorTickState();

	UE_LOG(LogSurvivorArena, Log, TEXT("ClearCards completed. Owner=%s"), *GetNameSafe(GetOwner()));
	EmitCardDebugMessage(TEXT("Cards Cleared"), FColor::Silver);
	BroadcastLoadoutChanged();
}

void USurvivorCardLoadoutComponent::NotifyRunStarted()
{
	for (const FSurvivorEquippedCardEntry& Entry : EquippedCardEntries)
	{
		if (!Entry.CardDefinition || Entry.StackCount <= 0)
		{
			continue;
		}

		const FSurvivorCardBehaviorContext Context = BuildBehaviorContext(Entry.CardDefinition, Entry.StackCount);
		for (USurvivorCardBehavior* Behavior : Entry.CardDefinition->Behaviors)
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
	for (const FSurvivorEquippedCardEntry& Entry : EquippedCardEntries)
	{
		if (!Entry.CardDefinition || Entry.StackCount <= 0)
		{
			continue;
		}

		FSurvivorCardEnemyKillContext Context;
		Context.CardContext = BuildBehaviorContext(Entry.CardDefinition, Entry.StackCount);
		Context.KilledEnemyActor = KilledEnemyActor;
		Context.KillCountDelta = KillCountDelta;

		for (USurvivorCardBehavior* Behavior : Entry.CardDefinition->Behaviors)
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
	int32 UsedSlots = 0;
	for (const FSurvivorEquippedCardEntry& Entry : EquippedCardEntries)
	{
		UsedSlots += Entry.StackCount;
	}
	return UsedSlots;
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

void USurvivorCardLoadoutComponent::RecalculateAggregatedCardTags()
{
	AggregatedCardTags.Reset();

	for (const FSurvivorEquippedCardEntry& Entry : EquippedCardEntries)
	{
		if (Entry.CardDefinition && Entry.StackCount > 0)
		{
			AggregatedCardTags.AppendTags(Entry.CardDefinition->CardTags);
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

int32 USurvivorCardLoadoutComponent::GetGrantedStartingWeaponCount() const
{
	return GrantedStartingWeapons.Num();
}

int32 USurvivorCardLoadoutComponent::GetGrantedStartingAbilitySetCount() const
{
	return GrantedStartingAbilitySetHandles.Num();
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

FSurvivorCardBehaviorContext USurvivorCardLoadoutComponent::BuildBehaviorContext(USurvivorCardDefinition* CardDefinition, int32 StackCount) const
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
	Context.StackCount = StackCount;
	return Context;
}

bool USurvivorCardLoadoutComponent::ApplyCardStack(USurvivorCardDefinition* CardDefinition, FSurvivorAppliedCardHandles& OutAppliedHandles)
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

	const FSurvivorCardBehaviorContext BehaviorContext = BuildBehaviorContext(CardDefinition, FindCardEntry(CardDefinition) ? FindCardEntry(CardDefinition)->StackCount + 1 : 1);
	for (USurvivorCardBehavior* Behavior : CardDefinition->Behaviors)
	{
		if (!Behavior)
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("Card %s contains a null Behavior entry."), *GetNameSafe(CardDefinition));
			continue;
		}

		if (!Behavior->OnCardEquipped(BehaviorContext, OutAppliedHandles))
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("Card behavior equip failed. Owner=%s Card=%s Behavior=%s"),
				*GetNameSafe(GetOwner()),
				*GetNameSafe(CardDefinition),
				*GetNameSafe(Behavior));
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
	for (const FSurvivorEquippedCardEntry& Entry : EquippedCardEntries)
	{
		if (!Entry.CardDefinition || Entry.StackCount <= 0)
		{
			continue;
		}

		for (USurvivorCardBehavior* Behavior : Entry.CardDefinition->Behaviors)
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

void USurvivorCardLoadoutComponent::BroadcastLoadoutChanged()
{
	LoadoutChangedEvent.Broadcast(this);
}

void USurvivorCardLoadoutComponent::BroadcastCardStackChanged(USurvivorCardDefinition* CardDefinition, int32 OldStackCount, int32 NewStackCount)
{
	if (!CardDefinition)
	{
		return;
	}

	const FSurvivorCardBehaviorContext Context = BuildBehaviorContext(CardDefinition, NewStackCount);
	for (USurvivorCardBehavior* Behavior : CardDefinition->Behaviors)
	{
		if (Behavior)
		{
			Behavior->OnCardStackChanged(Context, OldStackCount, NewStackCount);
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
