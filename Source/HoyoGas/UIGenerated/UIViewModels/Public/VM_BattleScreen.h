#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VM_BattleScreen.generated.h"

class UUIStoreBase;
class UBattleUIStore;

UCLASS(BlueprintType)
class HOYOGAS_API UVM_BattleScreen : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(UBattleUIStore* InBattleStore);
	void Teardown();

	FText GetCommandHintText() const;
	void SetCommandHintText(const FText& InValue);

	FText GetCurrentActorText() const;
	void SetCurrentActorText(const FText& InValue);

	FText GetEncounterTitleText() const;
	void SetEncounterTitleText(const FText& InValue);

	FText GetReticleDebugText() const;
	void SetReticleDebugText(const FText& InValue);

	FText GetTargetHintText() const;
	void SetTargetHintText(const FText& InValue);

	FText GetTargetAffinityText() const;
	void SetTargetAffinityText(const FText& InValue);

	FText GetTargetLevelText() const;
	void SetTargetLevelText(const FText& InValue);

	FText GetTargetNameText() const;
	void SetTargetNameText(const FText& InValue);

	FText GetTargetStateText() const;
	void SetTargetStateText(const FText& InValue);


private:
	void RefreshFromStore();
	void HandleStoreChanged(UUIStoreBase* ChangedStore);

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CommandHintText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CurrentActorText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText EncounterTitleText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText ReticleDebugText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText TargetHintText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText TargetAffinityText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText TargetLevelText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText TargetNameText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText TargetStateText;

	TWeakObjectPtr<UBattleUIStore> BattleStore;
};
