#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VM_HUD.generated.h"

class UInventoryUIStore;
class UPlayerVitalsUIStore;
class UProgressionUIStore;
class UUIStoreBase;

UCLASS(BlueprintType)
class HOYOGAS_API UVM_HUD : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(
		UProgressionUIStore* InProgressionStore,
		UInventoryUIStore* InInventoryStore,
		UPlayerVitalsUIStore* InPlayerVitalsStore);
	void Teardown();

	FText GetCurrentScoreText() const;
	void SetCurrentScoreText(const FText& InValue);

	FText GetCurrentLevelText() const;
	void SetCurrentLevelText(const FText& InValue);

	FText GetRemainingToNextLevelText() const;
	void SetRemainingToNextLevelText(const FText& InValue);

	float GetProgressPercent() const;
	void SetProgressPercent(float InValue);

	FText GetTotalInventoryValueText() const;
	void SetTotalInventoryValueText(const FText& InValue);

	FText GetHealthText() const;
	void SetHealthText(const FText& InValue);

	float GetHealthPercent() const;
	void SetHealthPercent(float InValue);

	FText GetEnergyText() const;
	void SetEnergyText(const FText& InValue);

	float GetEnergyPercent() const;
	void SetEnergyPercent(float InValue);

	FText GetEnergyRechargeText() const;
	void SetEnergyRechargeText(const FText& InValue);

private:
	void RefreshFromStores();
	void HandleStoreChanged(UUIStoreBase* ChangedStore);

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CurrentScoreText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CurrentLevelText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText RemainingToNextLevelText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	float ProgressPercent = 0.0f;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText TotalInventoryValueText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText HealthText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	float HealthPercent = 0.0f;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText EnergyText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	float EnergyPercent = 0.0f;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText EnergyRechargeText;

	TWeakObjectPtr<UProgressionUIStore> ProgressionStore;
	TWeakObjectPtr<UInventoryUIStore> InventoryStore;
	TWeakObjectPtr<UPlayerVitalsUIStore> PlayerVitalsStore;
};
