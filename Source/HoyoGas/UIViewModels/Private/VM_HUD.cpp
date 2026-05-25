#include "VM_HUD.h"

#include "Stores/InventoryUIStore.h"
#include "Stores/PlayerVitalsUIStore.h"
#include "Stores/ProgressionUIStore.h"

void UVM_HUD::Initialize(
	UProgressionUIStore* InProgressionStore,
	UInventoryUIStore* InInventoryStore,
	UPlayerVitalsUIStore* InPlayerVitalsStore)
{
	if (ProgressionStore.Get() != InProgressionStore)
	{
		if (ProgressionStore.IsValid())
		{
			ProgressionStore->OnStoreChanged().RemoveAll(this);
		}

		ProgressionStore = InProgressionStore;
		if (ProgressionStore.IsValid())
		{
			ProgressionStore->OnStoreChanged().AddUObject(this, &UVM_HUD::HandleStoreChanged);
		}
	}

	if (InventoryStore.Get() != InInventoryStore)
	{
		if (InventoryStore.IsValid())
		{
			InventoryStore->OnStoreChanged().RemoveAll(this);
		}

		InventoryStore = InInventoryStore;
		if (InventoryStore.IsValid())
		{
			InventoryStore->OnStoreChanged().AddUObject(this, &UVM_HUD::HandleStoreChanged);
		}
	}

	if (PlayerVitalsStore.Get() != InPlayerVitalsStore)
	{
		if (PlayerVitalsStore.IsValid())
		{
			PlayerVitalsStore->OnStoreChanged().RemoveAll(this);
		}

		PlayerVitalsStore = InPlayerVitalsStore;
		if (PlayerVitalsStore.IsValid())
		{
			PlayerVitalsStore->OnStoreChanged().AddUObject(this, &UVM_HUD::HandleStoreChanged);
		}
	}

	RefreshFromStores();
}

void UVM_HUD::Teardown()
{
	if (ProgressionStore.IsValid())
	{
		ProgressionStore->OnStoreChanged().RemoveAll(this);
		ProgressionStore.Reset();
	}

	if (InventoryStore.IsValid())
	{
		InventoryStore->OnStoreChanged().RemoveAll(this);
		InventoryStore.Reset();
	}

	if (PlayerVitalsStore.IsValid())
	{
		PlayerVitalsStore->OnStoreChanged().RemoveAll(this);
		PlayerVitalsStore.Reset();
	}
}

FText UVM_HUD::GetCurrentScoreText() const
{
	return CurrentScoreText;
}

void UVM_HUD::SetCurrentScoreText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentScoreText, InValue);
}

FText UVM_HUD::GetCurrentLevelText() const
{
	return CurrentLevelText;
}

void UVM_HUD::SetCurrentLevelText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentLevelText, InValue);
}

FText UVM_HUD::GetRemainingToNextLevelText() const
{
	return RemainingToNextLevelText;
}

void UVM_HUD::SetRemainingToNextLevelText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(RemainingToNextLevelText, InValue);
}

float UVM_HUD::GetProgressPercent() const
{
	return ProgressPercent;
}

void UVM_HUD::SetProgressPercent(float InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ProgressPercent, InValue);
}

FText UVM_HUD::GetTotalInventoryValueText() const
{
	return TotalInventoryValueText;
}

void UVM_HUD::SetTotalInventoryValueText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(TotalInventoryValueText, InValue);
}

FText UVM_HUD::GetHealthText() const
{
	return HealthText;
}

void UVM_HUD::SetHealthText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(HealthText, InValue);
}

float UVM_HUD::GetHealthPercent() const
{
	return HealthPercent;
}

void UVM_HUD::SetHealthPercent(float InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(HealthPercent, InValue);
}

FText UVM_HUD::GetEnergyText() const
{
	return EnergyText;
}

void UVM_HUD::SetEnergyText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(EnergyText, InValue);
}

float UVM_HUD::GetEnergyPercent() const
{
	return EnergyPercent;
}

void UVM_HUD::SetEnergyPercent(float InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(EnergyPercent, InValue);
}

FText UVM_HUD::GetEnergyRechargeText() const
{
	return EnergyRechargeText;
}

void UVM_HUD::SetEnergyRechargeText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(EnergyRechargeText, InValue);
}

void UVM_HUD::RefreshFromStores()
{
	const int32 CurrentScore = ProgressionStore.IsValid() ? ProgressionStore->GetCurrentScore() : 0;
	const int32 CurrentLevel = ProgressionStore.IsValid() ? ProgressionStore->GetCurrentLevel() : 1;
	const int32 RemainingScore = ProgressionStore.IsValid() ? ProgressionStore->GetScoreToNextLevel() : 0;
	const float ProgressValue = ProgressionStore.IsValid() ? ProgressionStore->GetProgressToNextLevel01() : 0.0f;
	const int32 TotalInventoryValue = InventoryStore.IsValid() ? InventoryStore->GetTotalValue() : 0;

	SetCurrentScoreText(FText::Format(INVTEXT("Score: {0}"), FText::AsNumber(CurrentScore)));
	SetCurrentLevelText(FText::Format(INVTEXT("Level: {0}"), FText::AsNumber(CurrentLevel)));
	SetRemainingToNextLevelText(FText::Format(INVTEXT("Next level in {0}"), FText::AsNumber(RemainingScore)));
	SetProgressPercent(ProgressValue);
	SetTotalInventoryValueText(FText::Format(INVTEXT("Inventory Value: {0}"), FText::AsNumber(TotalInventoryValue)));

	SetHealthText(PlayerVitalsStore.IsValid() ? PlayerVitalsStore->GetHealthText() : FText::GetEmpty());
	SetHealthPercent(PlayerVitalsStore.IsValid() ? PlayerVitalsStore->GetHealthPercent() : 0.0f);
	SetEnergyText(PlayerVitalsStore.IsValid() ? PlayerVitalsStore->GetEnergyText() : FText::GetEmpty());
	SetEnergyPercent(PlayerVitalsStore.IsValid() ? PlayerVitalsStore->GetEnergyPercent() : 0.0f);
	SetEnergyRechargeText(PlayerVitalsStore.IsValid() ? PlayerVitalsStore->GetEnergyRechargeText() : FText::GetEmpty());
}

void UVM_HUD::HandleStoreChanged(UUIStoreBase* ChangedStore)
{
	RefreshFromStores();
}
