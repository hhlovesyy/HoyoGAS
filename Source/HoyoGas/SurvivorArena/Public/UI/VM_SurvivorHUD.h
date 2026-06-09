#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VM_SurvivorHUD.generated.h"

class UUIStoreBase;
class USurvivorHUDStore;

UCLASS(BlueprintType)
class HOYOGAS_API UVM_SurvivorHUD : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(USurvivorHUDStore* InHUDStore);
	void Teardown();

	FText GetHealthText() const;
	void SetHealthText(const FText& InValue);

	float GetHealthPercent() const;
	void SetHealthPercent(float InValue);

	FText GetLevelText() const;
	void SetLevelText(const FText& InValue);

	FText GetWeaponCountText() const;
	void SetWeaponCountText(const FText& InValue);

	FText GetGoldText() const;
	void SetGoldText(const FText& InValue);

	FText GetExperienceText() const;
	void SetExperienceText(const FText& InValue);

	FText GetCardSlotsText() const;
	void SetCardSlotsText(const FText& InValue);

	FText GetEquippedCardsText() const;
	void SetEquippedCardsText(const FText& InValue);

	FText GetCardTagsText() const;
	void SetCardTagsText(const FText& InValue);

private:
	void RefreshFromStore();
	void HandleStoreChanged(UUIStoreBase* ChangedStore);

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText HealthText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	float HealthPercent = 0.0f;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText LevelText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText WeaponCountText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText GoldText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText ExperienceText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CardSlotsText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText EquippedCardsText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CardTagsText;

	TWeakObjectPtr<USurvivorHUDStore> HUDStore;
};
