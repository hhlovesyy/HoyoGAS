#include "Cards/SurvivorCardLoadoutComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Cards/SurvivorCardDefinition.h"
#include "Core/SurvivorArenaLog.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameplayEffect.h"

bool FSurvivorAppliedCardHandles::IsEmpty() const
{
	return ActiveEffectHandles.IsEmpty() && GrantedAbilitySpecHandles.IsEmpty();
}

void FSurvivorAppliedCardHandles::Reset()
{
	ActiveEffectHandles.Reset();
	GrantedAbilitySpecHandles.Reset();
}

USurvivorCardLoadoutComponent::USurvivorCardLoadoutComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USurvivorCardLoadoutComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogSurvivorArena, Log, TEXT("CardLoadoutComponent initialized. Owner=%s MaxCardSlots=%d"), *GetNameSafe(GetOwner()), MaxCardSlots);
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

	UE_LOG(LogSurvivorArena, Log, TEXT("EquipCard succeeded. Owner=%s Card=%s UsedSlots=%d/%d"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(CardDefinition),
		GetUsedCardSlots(),
		MaxCardSlots);

	EmitCardDebugMessage(FString::Printf(TEXT("Card Equipped: %s (%d/%d)"), *GetNameSafe(CardDefinition), GetUsedCardSlots(), MaxCardSlots), FColor::Green);
	PrintCardTagSummary();
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
			RemoveAppliedHandles(AppliedHandles);
		}

		Entry.StackCount = FMath::Max(0, Entry.StackCount - 1);
		if (Entry.StackCount == 0)
		{
			EquippedCardEntries.RemoveAt(Index);
		}

		RecalculateAggregatedCardTags();

		UE_LOG(LogSurvivorArena, Log, TEXT("UnequipCard succeeded. Owner=%s Card=%s UsedSlots=%d/%d"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(CardDefinition),
			GetUsedCardSlots(),
			MaxCardSlots);

		EmitCardDebugMessage(FString::Printf(TEXT("Card Unequipped: %s (%d/%d)"), *GetNameSafe(CardDefinition), GetUsedCardSlots(), MaxCardSlots), FColor::Yellow);
		PrintCardTagSummary();
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

	UE_LOG(LogSurvivorArena, Log, TEXT("ClearCards completed. Owner=%s"), *GetNameSafe(GetOwner()));
	EmitCardDebugMessage(TEXT("Cards Cleared"), FColor::Silver);
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

UAbilitySystemComponent* USurvivorCardLoadoutComponent::ResolveOwnerASC() const
{
	if (const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		return AbilitySystemInterface->GetAbilitySystemComponent();
	}

	return nullptr;
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
