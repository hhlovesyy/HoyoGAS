#include "UI/VM_SurvivorHUD.h"

#include "UI/SurvivorHUDStore.h"

void UVM_SurvivorHUD::Initialize(USurvivorHUDStore* InHUDStore)
{
	if (HUDStore.Get() != InHUDStore)
	{
		if (HUDStore.IsValid())
		{
			HUDStore->OnStoreChanged().RemoveAll(this);
		}

		HUDStore = InHUDStore;
		if (HUDStore.IsValid())
		{
			HUDStore->OnStoreChanged().AddUObject(this, &UVM_SurvivorHUD::HandleStoreChanged);
		}
	}

	RefreshFromStore();
}

void UVM_SurvivorHUD::Teardown()
{
	if (HUDStore.IsValid())
	{
		HUDStore->OnStoreChanged().RemoveAll(this);
		HUDStore.Reset();
	}
}

FText UVM_SurvivorHUD::GetHealthText() const { return HealthText; }
void UVM_SurvivorHUD::SetHealthText(const FText& InValue) { UE_MVVM_SET_PROPERTY_VALUE(HealthText, InValue); }
float UVM_SurvivorHUD::GetHealthPercent() const { return HealthPercent; }
void UVM_SurvivorHUD::SetHealthPercent(float InValue) { UE_MVVM_SET_PROPERTY_VALUE(HealthPercent, InValue); }
FText UVM_SurvivorHUD::GetLevelText() const { return LevelText; }
void UVM_SurvivorHUD::SetLevelText(const FText& InValue) { UE_MVVM_SET_PROPERTY_VALUE(LevelText, InValue); }
int32 UVM_SurvivorHUD::GetSurvivorLevelValue() const { return SurvivorLevelValue; }
void UVM_SurvivorHUD::SetSurvivorLevelValue(int32 InValue) { UE_MVVM_SET_PROPERTY_VALUE(SurvivorLevelValue, InValue); }
FText UVM_SurvivorHUD::GetWeaponCountText() const { return WeaponCountText; }
void UVM_SurvivorHUD::SetWeaponCountText(const FText& InValue) { UE_MVVM_SET_PROPERTY_VALUE(WeaponCountText, InValue); }
FText UVM_SurvivorHUD::GetGoldText() const { return GoldText; }
void UVM_SurvivorHUD::SetGoldText(const FText& InValue) { UE_MVVM_SET_PROPERTY_VALUE(GoldText, InValue); }
FText UVM_SurvivorHUD::GetExperienceText() const { return ExperienceText; }
void UVM_SurvivorHUD::SetExperienceText(const FText& InValue) { UE_MVVM_SET_PROPERTY_VALUE(ExperienceText, InValue); }
float UVM_SurvivorHUD::GetCurrentExperienceValue() const { return CurrentExperienceValue; }
void UVM_SurvivorHUD::SetCurrentExperienceValue(float InValue) { UE_MVVM_SET_PROPERTY_VALUE(CurrentExperienceValue, InValue); }
bool UVM_SurvivorHUD::GetHasExperienceProgressTargetValue() const { return HasExperienceProgressTargetValue; }
void UVM_SurvivorHUD::SetHasExperienceProgressTargetValue(bool InValue) { UE_MVVM_SET_PROPERTY_VALUE(HasExperienceProgressTargetValue, InValue); }
float UVM_SurvivorHUD::GetExperienceProgressTargetValue() const { return ExperienceProgressTargetValue; }
void UVM_SurvivorHUD::SetExperienceProgressTargetValue(float InValue) { UE_MVVM_SET_PROPERTY_VALUE(ExperienceProgressTargetValue, InValue); }
float UVM_SurvivorHUD::GetExperienceProgressPercent() const { return ExperienceProgressPercent; }
void UVM_SurvivorHUD::SetExperienceProgressPercent(float InValue) { UE_MVVM_SET_PROPERTY_VALUE(ExperienceProgressPercent, InValue); }
int32 UVM_SurvivorHUD::GetGoldDelta() const { return GoldDelta; }
void UVM_SurvivorHUD::SetGoldDelta(int32 InValue) { UE_MVVM_SET_PROPERTY_VALUE(GoldDelta, InValue); }
float UVM_SurvivorHUD::GetExperienceDelta() const { return ExperienceDelta; }
void UVM_SurvivorHUD::SetExperienceDelta(float InValue) { UE_MVVM_SET_PROPERTY_VALUE(ExperienceDelta, InValue); }
int32 UVM_SurvivorHUD::GetGoldChangeEventId() const { return GoldChangeEventId; }
void UVM_SurvivorHUD::SetGoldChangeEventId(int32 InValue) { UE_MVVM_SET_PROPERTY_VALUE(GoldChangeEventId, InValue); }
int32 UVM_SurvivorHUD::GetExperienceChangeEventId() const { return ExperienceChangeEventId; }
void UVM_SurvivorHUD::SetExperienceChangeEventId(int32 InValue) { UE_MVVM_SET_PROPERTY_VALUE(ExperienceChangeEventId, InValue); }
float UVM_SurvivorHUD::GetExperienceLevelUpClearedAmount() const { return ExperienceLevelUpClearedAmount; }
void UVM_SurvivorHUD::SetExperienceLevelUpClearedAmount(float InValue) { UE_MVVM_SET_PROPERTY_VALUE(ExperienceLevelUpClearedAmount, InValue); }
int32 UVM_SurvivorHUD::GetExperienceLevelUpEventId() const { return ExperienceLevelUpEventId; }
void UVM_SurvivorHUD::SetExperienceLevelUpEventId(int32 InValue) { UE_MVVM_SET_PROPERTY_VALUE(ExperienceLevelUpEventId, InValue); }
FText UVM_SurvivorHUD::GetCardSlotsText() const { return CardSlotsText; }
void UVM_SurvivorHUD::SetCardSlotsText(const FText& InValue) { UE_MVVM_SET_PROPERTY_VALUE(CardSlotsText, InValue); }
FText UVM_SurvivorHUD::GetEquippedCardsText() const { return EquippedCardsText; }
void UVM_SurvivorHUD::SetEquippedCardsText(const FText& InValue) { UE_MVVM_SET_PROPERTY_VALUE(EquippedCardsText, InValue); }
FText UVM_SurvivorHUD::GetCardTagsText() const { return CardTagsText; }
void UVM_SurvivorHUD::SetCardTagsText(const FText& InValue) { UE_MVVM_SET_PROPERTY_VALUE(CardTagsText, InValue); }

void UVM_SurvivorHUD::RefreshFromStore()
{
	SetHealthText(HUDStore.IsValid() ? HUDStore->GetHealthText() : FText::GetEmpty());
	SetHealthPercent(HUDStore.IsValid() ? HUDStore->GetHealthPercent() : 0.0f);
	SetLevelText(HUDStore.IsValid() ? HUDStore->GetLevelText() : FText::GetEmpty());
	SetSurvivorLevelValue(HUDStore.IsValid() ? HUDStore->GetSurvivorLevelValue() : 1);
	SetWeaponCountText(HUDStore.IsValid() ? HUDStore->GetWeaponCountText() : FText::GetEmpty());
	SetGoldText(HUDStore.IsValid() ? HUDStore->GetGoldText() : FText::GetEmpty());
	SetExperienceText(HUDStore.IsValid() ? HUDStore->GetExperienceText() : FText::GetEmpty());
	SetCurrentExperienceValue(HUDStore.IsValid() ? HUDStore->GetCurrentExperienceValue() : 0.0f);
	SetHasExperienceProgressTargetValue(HUDStore.IsValid() ? HUDStore->HasExperienceProgressTarget() : false);
	SetExperienceProgressTargetValue(HUDStore.IsValid() ? HUDStore->GetExperienceProgressTargetValue() : 0.0f);
	SetExperienceProgressPercent(HUDStore.IsValid() ? HUDStore->GetExperienceProgressPercent() : 0.0f);
	SetGoldDelta(HUDStore.IsValid() ? HUDStore->GetGoldDelta() : 0);
	SetExperienceDelta(HUDStore.IsValid() ? HUDStore->GetExperienceDelta() : 0.0f);
	SetGoldChangeEventId(HUDStore.IsValid() ? HUDStore->GetGoldChangeEventId() : 0);
	SetExperienceChangeEventId(HUDStore.IsValid() ? HUDStore->GetExperienceChangeEventId() : 0);
	SetExperienceLevelUpClearedAmount(HUDStore.IsValid() ? HUDStore->GetExperienceLevelUpClearedAmount() : 0.0f);
	SetExperienceLevelUpEventId(HUDStore.IsValid() ? HUDStore->GetExperienceLevelUpEventId() : 0);
	SetCardSlotsText(HUDStore.IsValid() ? HUDStore->GetCardSlotsText() : FText::GetEmpty());
	SetEquippedCardsText(HUDStore.IsValid() ? HUDStore->GetEquippedCardsText() : FText::GetEmpty());
	SetCardTagsText(HUDStore.IsValid() ? HUDStore->GetCardTagsText() : FText::GetEmpty());
}

void UVM_SurvivorHUD::HandleStoreChanged(UUIStoreBase* ChangedStore)
{
	RefreshFromStore();
}
