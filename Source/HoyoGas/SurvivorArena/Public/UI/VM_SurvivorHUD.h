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

	int32 GetSurvivorLevelValue() const;
	void SetSurvivorLevelValue(int32 InValue);

	FText GetWeaponCountText() const;
	void SetWeaponCountText(const FText& InValue);

	FText GetGoldText() const;
	void SetGoldText(const FText& InValue);

	FText GetExperienceText() const;
	void SetExperienceText(const FText& InValue);

	float GetCurrentExperienceValue() const;
	void SetCurrentExperienceValue(float InValue);

	bool GetHasExperienceProgressTargetValue() const;
	void SetHasExperienceProgressTargetValue(bool InValue);

	float GetExperienceProgressTargetValue() const;
	void SetExperienceProgressTargetValue(float InValue);

	float GetExperienceProgressPercent() const;
	void SetExperienceProgressPercent(float InValue);

	int32 GetGoldDelta() const;
	void SetGoldDelta(int32 InValue);

	float GetExperienceDelta() const;
	void SetExperienceDelta(float InValue);

	int32 GetGoldChangeEventId() const;
	void SetGoldChangeEventId(int32 InValue);

	int32 GetExperienceChangeEventId() const;
	void SetExperienceChangeEventId(int32 InValue);

	float GetExperienceLevelUpClearedAmount() const;
	void SetExperienceLevelUpClearedAmount(float InValue);

	int32 GetExperienceLevelUpEventId() const;
	void SetExperienceLevelUpEventId(int32 InValue);

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
	int32 SurvivorLevelValue = 1;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText WeaponCountText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText GoldText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText ExperienceText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	float CurrentExperienceValue = 0.0f;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	bool HasExperienceProgressTargetValue = false;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	float ExperienceProgressTargetValue = 0.0f;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	float ExperienceProgressPercent = 0.0f;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	int32 GoldDelta = 0;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	float ExperienceDelta = 0.0f;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	int32 GoldChangeEventId = 0;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	int32 ExperienceChangeEventId = 0;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	float ExperienceLevelUpClearedAmount = 0.0f;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	int32 ExperienceLevelUpEventId = 0;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CardSlotsText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText EquippedCardsText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CardTagsText;

	TWeakObjectPtr<USurvivorHUDStore> HUDStore;
};
