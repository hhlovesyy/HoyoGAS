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
FText UVM_SurvivorHUD::GetWeaponCountText() const { return WeaponCountText; }
void UVM_SurvivorHUD::SetWeaponCountText(const FText& InValue) { UE_MVVM_SET_PROPERTY_VALUE(WeaponCountText, InValue); }
FText UVM_SurvivorHUD::GetGoldText() const { return GoldText; }
void UVM_SurvivorHUD::SetGoldText(const FText& InValue) { UE_MVVM_SET_PROPERTY_VALUE(GoldText, InValue); }
FText UVM_SurvivorHUD::GetExperienceText() const { return ExperienceText; }
void UVM_SurvivorHUD::SetExperienceText(const FText& InValue) { UE_MVVM_SET_PROPERTY_VALUE(ExperienceText, InValue); }
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
	SetWeaponCountText(HUDStore.IsValid() ? HUDStore->GetWeaponCountText() : FText::GetEmpty());
	SetGoldText(HUDStore.IsValid() ? HUDStore->GetGoldText() : FText::GetEmpty());
	SetExperienceText(HUDStore.IsValid() ? HUDStore->GetExperienceText() : FText::GetEmpty());
	SetCardSlotsText(HUDStore.IsValid() ? HUDStore->GetCardSlotsText() : FText::GetEmpty());
	SetEquippedCardsText(HUDStore.IsValid() ? HUDStore->GetEquippedCardsText() : FText::GetEmpty());
	SetCardTagsText(HUDStore.IsValid() ? HUDStore->GetCardTagsText() : FText::GetEmpty());
}

void UVM_SurvivorHUD::HandleStoreChanged(UUIStoreBase* ChangedStore)
{
	RefreshFromStore();
}
