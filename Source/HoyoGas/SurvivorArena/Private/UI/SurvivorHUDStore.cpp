#include "UI/SurvivorHUDStore.h"

#include "AbilitySystemComponent.h"
#include "Cards/SurvivorCardLoadoutComponent.h"
#include "Core/SurvivorArenaLog.h"
#include "Economy/SurvivorRunEconomyComponent.h"
#include "GAS/SurvivorAttributeSet.h"
#include "Player/SurvivorCharacter.h"
#include "Player/SurvivorPlayerState.h"
#include "Weapons/SurvivorWeaponManagerComponent.h"

void USurvivorHUDStore::BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState)
{
	Super::BindToPlayerContext(InPawn, InPlayerState);

	ASurvivorPlayerState* SurvivorPlayerState = Cast<ASurvivorPlayerState>(InPlayerState);
	ASurvivorCharacter* SurvivorCharacter = Cast<ASurvivorCharacter>(InPawn);

	BindAbilitySystem(SurvivorPlayerState ? SurvivorPlayerState->GetAbilitySystemComponent() : nullptr);
	BindLoadoutComponent(SurvivorPlayerState ? SurvivorPlayerState->GetLoadoutComponent() : nullptr);
	BindRunEconomyComponent(SurvivorPlayerState ? SurvivorPlayerState->GetRunEconomyComponent() : nullptr);
	BindWeaponManager(SurvivorCharacter ? SurvivorCharacter->GetWeaponManagerComponent() : nullptr);

	UE_LOG(LogSurvivorArena, Log, TEXT("SurvivorHUDStore bound to player context. Pawn=%s PlayerState=%s ASC=%s Loadout=%s Economy=%s WeaponManager=%s"),
		*GetNameSafe(InPawn),
		*GetNameSafe(InPlayerState),
		*GetNameSafe(SurvivorPlayerState ? SurvivorPlayerState->GetAbilitySystemComponent() : nullptr),
		*GetNameSafe(SurvivorPlayerState ? SurvivorPlayerState->GetLoadoutComponent() : nullptr),
		*GetNameSafe(SurvivorPlayerState ? SurvivorPlayerState->GetRunEconomyComponent() : nullptr),
		*GetNameSafe(SurvivorCharacter ? SurvivorCharacter->GetWeaponManagerComponent() : nullptr));

	RefreshAll();
}

void USurvivorHUDStore::UnbindFromPlayerContext()
{
	UnbindWeaponManager();
	UnbindRunEconomyComponent();
	UnbindLoadoutComponent();
	UnbindAbilitySystem();
	Super::UnbindFromPlayerContext();
	BroadcastStoreChanged();
}

const FText& USurvivorHUDStore::GetHealthText() const
{
	return HealthText;
}

float USurvivorHUDStore::GetHealthPercent() const
{
	return HealthPercent;
}

const FText& USurvivorHUDStore::GetLevelText() const
{
	return LevelText;
}

int32 USurvivorHUDStore::GetSurvivorLevelValue() const
{
	return CachedSurvivorLevel;
}

const FText& USurvivorHUDStore::GetWeaponCountText() const
{
	return WeaponCountText;
}

const FText& USurvivorHUDStore::GetGoldText() const
{
	return GoldText;
}

const FText& USurvivorHUDStore::GetExperienceText() const
{
	return ExperienceText;
}

float USurvivorHUDStore::GetCurrentExperienceValue() const
{
	return CachedExperience;
}

bool USurvivorHUDStore::HasExperienceProgressTarget() const
{
	return bHasExperienceProgressTarget;
}

float USurvivorHUDStore::GetExperienceProgressTargetValue() const
{
	return ExperienceProgressTargetValue;
}

float USurvivorHUDStore::GetExperienceProgressPercent() const
{
	return ExperienceProgressPercent;
}

int32 USurvivorHUDStore::GetGoldDelta() const
{
	return GoldDelta;
}

float USurvivorHUDStore::GetExperienceDelta() const
{
	return ExperienceDelta;
}

int32 USurvivorHUDStore::GetGoldChangeEventId() const
{
	return GoldChangeEventId;
}

int32 USurvivorHUDStore::GetExperienceChangeEventId() const
{
	return ExperienceChangeEventId;
}

float USurvivorHUDStore::GetExperienceLevelUpClearedAmount() const
{
	return ExperienceLevelUpClearedAmount;
}

int32 USurvivorHUDStore::GetExperienceLevelUpEventId() const
{
	return ExperienceLevelUpEventId;
}

const FText& USurvivorHUDStore::GetCardSlotsText() const
{
	return CardSlotsText;
}

const FText& USurvivorHUDStore::GetEquippedCardsText() const
{
	return EquippedCardsText;
}

const FText& USurvivorHUDStore::GetCardTagsText() const
{
	return CardTagsText;
}

void USurvivorHUDStore::BindAbilitySystem(UAbilitySystemComponent* InAbilitySystemComponent)
{
	if (AbilitySystemComponent.Get() == InAbilitySystemComponent)
	{
		return;
	}

	UnbindAbilitySystem();
	AbilitySystemComponent = InAbilitySystemComponent;

	if (AbilitySystemComponent.IsValid())
	{
		UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
		HealthChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(USurvivorAttributeSet::GetHealthAttribute())
			.AddUObject(this, &USurvivorHUDStore::HandleHealthChanged);
		MaxHealthChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(USurvivorAttributeSet::GetMaxHealthAttribute())
			.AddUObject(this, &USurvivorHUDStore::HandleMaxHealthChanged);
	}
}

void USurvivorHUDStore::UnbindAbilitySystem()
{
	if (AbilitySystemComponent.IsValid())
	{
		UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
		if (HealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(USurvivorAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
		}

		if (MaxHealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(USurvivorAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
		}
	}

	AbilitySystemComponent.Reset();
	HealthChangedHandle.Reset();
	MaxHealthChangedHandle.Reset();
	Health = 0.0f;
	MaxHealth = 0.0f;
}

void USurvivorHUDStore::BindLoadoutComponent(USurvivorCardLoadoutComponent* InLoadoutComponent)
{
	if (LoadoutComponent.Get() == InLoadoutComponent)
	{
		return;
	}

	UnbindLoadoutComponent();
	LoadoutComponent = InLoadoutComponent;

	if (LoadoutComponent.IsValid())
	{
		LoadoutComponent->OnLoadoutChanged().AddUObject(this, &USurvivorHUDStore::HandleLoadoutChanged);
	}
}

void USurvivorHUDStore::UnbindLoadoutComponent()
{
	if (LoadoutComponent.IsValid())
	{
		LoadoutComponent->OnLoadoutChanged().RemoveAll(this);
	}

	LoadoutComponent.Reset();
}

void USurvivorHUDStore::BindRunEconomyComponent(USurvivorRunEconomyComponent* InRunEconomyComponent)
{
	if (RunEconomyComponent.Get() == InRunEconomyComponent)
	{
		return;
	}

	UnbindRunEconomyComponent();
	RunEconomyComponent = InRunEconomyComponent;

	if (RunEconomyComponent.IsValid())
	{
		RunEconomyComponent->OnEconomyChanged().AddUObject(this, &USurvivorHUDStore::HandleEconomyChanged);
	}
}

void USurvivorHUDStore::UnbindRunEconomyComponent()
{
	if (RunEconomyComponent.IsValid())
	{
		RunEconomyComponent->OnEconomyChanged().RemoveAll(this);
	}

	RunEconomyComponent.Reset();
	CachedSurvivorLevel = 1;
	CachedGold = 0;
	CachedExperience = 0.0f;
	bHasExperienceProgressTarget = false;
	ExperienceProgressTargetValue = 0.0f;
	ExperienceProgressPercent = 0.0f;
	GoldDelta = 0;
	ExperienceDelta = 0.0f;
	ExperienceLevelUpClearedAmount = 0.0f;
	bHasEconomySnapshot = false;
}

void USurvivorHUDStore::BindWeaponManager(USurvivorWeaponManagerComponent* InWeaponManager)
{
	if (WeaponManagerComponent.Get() == InWeaponManager)
	{
		return;
	}

	UnbindWeaponManager();
	WeaponManagerComponent = InWeaponManager;

	if (WeaponManagerComponent.IsValid())
	{
		WeaponManagerComponent->OnWeaponsChanged().AddUObject(this, &USurvivorHUDStore::HandleWeaponsChanged);
	}
}

void USurvivorHUDStore::UnbindWeaponManager()
{
	if (WeaponManagerComponent.IsValid())
	{
		WeaponManagerComponent->OnWeaponsChanged().RemoveAll(this);
	}

	WeaponManagerComponent.Reset();
}

void USurvivorHUDStore::RefreshAll()
{
	if (const UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		Health = ASC->GetNumericAttribute(USurvivorAttributeSet::GetHealthAttribute());
		MaxHealth = ASC->GetNumericAttribute(USurvivorAttributeSet::GetMaxHealthAttribute());
	}
	else
	{
		Health = 0.0f;
		MaxHealth = 0.0f;
	}

	RefreshEconomySnapshot(false);
	RebuildDisplayValues();
	UE_LOG(LogSurvivorArena, Log, TEXT("SurvivorHUDStore refreshed. Health=%.2f MaxHealth=%.2f WeaponsText='%s' CardsText='%s'"),
		Health,
		MaxHealth,
		*WeaponCountText.ToString(),
		*CardSlotsText.ToString());
	BroadcastStoreChanged();
}

void USurvivorHUDStore::RefreshEconomySnapshot(bool bAllowDeltaEvents)
{
	const ASurvivorPlayerState* SurvivorPlayerState = Cast<ASurvivorPlayerState>(ObservedPlayerState.Get());
	const int32 NewSurvivorLevel = SurvivorPlayerState ? SurvivorPlayerState->GetSurvivorLevel() : 1;
	const int32 NewGold = RunEconomyComponent.IsValid() ? RunEconomyComponent->GetGold() : 0;
	const float NewExperience = RunEconomyComponent.IsValid() ? RunEconomyComponent->GetExperience() : 0.0f;

	if (!bHasEconomySnapshot || !bAllowDeltaEvents)
	{
		CachedSurvivorLevel = NewSurvivorLevel;
		CachedGold = NewGold;
		CachedExperience = NewExperience;
		bHasExperienceProgressTarget = false;
		ExperienceProgressTargetValue = 0.0f;
		ExperienceProgressPercent = 0.0f;
		GoldDelta = 0;
		ExperienceDelta = 0.0f;
		ExperienceLevelUpClearedAmount = 0.0f;
		bHasEconomySnapshot = true;
		return;
	}

	const bool bLevelIncreased = NewSurvivorLevel > CachedSurvivorLevel;
	bHasExperienceProgressTarget = false;
	ExperienceProgressTargetValue = 0.0f;
	ExperienceProgressPercent = 0.0f;
	GoldDelta = NewGold - CachedGold;
	ExperienceDelta = NewExperience - CachedExperience;
	ExperienceLevelUpClearedAmount = 0.0f;

	if (GoldDelta != 0)
	{
		++GoldChangeEventId;
	}

	if (bLevelIncreased)
	{
		ExperienceLevelUpClearedAmount = CachedExperience;
		++ExperienceLevelUpEventId;
	}
	else if (!FMath::IsNearlyZero(ExperienceDelta))
	{
		++ExperienceChangeEventId;
	}

	CachedSurvivorLevel = NewSurvivorLevel;
	CachedGold = NewGold;
	CachedExperience = NewExperience;
}

void USurvivorHUDStore::RebuildDisplayValues()
{
	HealthPercent = MaxHealth > 0.0f ? FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f) : 0.0f;
	HealthText = FText::Format(
		INVTEXT("{0} / {1}"),
		FText::AsNumber(FMath::RoundToInt(Health)),
		FText::AsNumber(FMath::RoundToInt(MaxHealth)));

	LevelText = FText::Format(INVTEXT("Lv. {0}"), FText::AsNumber(CachedSurvivorLevel));

	const int32 WeaponCount = WeaponManagerComponent.IsValid() ? WeaponManagerComponent->GetGrantedWeaponCount() : 0;
	WeaponCountText = FText::Format(INVTEXT("Weapons: {0}"), FText::AsNumber(WeaponCount));
	GoldText = FText::Format(INVTEXT("Gold: {0}"), FText::AsNumber(CachedGold));
	ExperienceText = FText::Format(INVTEXT("XP: {0}"), FText::AsNumber(FMath::RoundToInt(CachedExperience)));

	const int32 UsedCardSlots = LoadoutComponent.IsValid() ? LoadoutComponent->GetUsedCardSlots() : 0;
	const int32 MaxSlots = LoadoutComponent.IsValid() ? LoadoutComponent->GetMaxCardSlots() : 0;
	CardSlotsText = FText::Format(INVTEXT("Cards: {0} / {1}"), FText::AsNumber(UsedCardSlots), FText::AsNumber(MaxSlots));
	EquippedCardsText = LoadoutComponent.IsValid() ? LoadoutComponent->GetEquippedCardsSummaryText() : INVTEXT("<None>");
	CardTagsText = LoadoutComponent.IsValid() ? LoadoutComponent->GetAggregatedCardTagsSummaryText() : INVTEXT("<None>");
}

void USurvivorHUDStore::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	Health = Data.NewValue;
	RebuildDisplayValues();
	BroadcastStoreChanged();
}

void USurvivorHUDStore::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	MaxHealth = Data.NewValue;
	RebuildDisplayValues();
	BroadcastStoreChanged();
}

void USurvivorHUDStore::HandleLoadoutChanged(USurvivorCardLoadoutComponent* ChangedComponent)
{
	RebuildDisplayValues();
	BroadcastStoreChanged();
}

void USurvivorHUDStore::HandleEconomyChanged(USurvivorRunEconomyComponent* ChangedComponent)
{
	RefreshEconomySnapshot(true);
	RebuildDisplayValues();
	BroadcastStoreChanged();
}

void USurvivorHUDStore::HandleWeaponsChanged(USurvivorWeaponManagerComponent* ChangedComponent)
{
	RebuildDisplayValues();
	BroadcastStoreChanged();
}
