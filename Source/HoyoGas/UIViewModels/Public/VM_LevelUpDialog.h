#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VM_LevelUpDialog.generated.h"

class UProgressionUIStore;

UCLASS(BlueprintType)
class HOYOGAS_API UVM_LevelUpDialog : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(UProgressionUIStore* InProgressionStore);

	FText GetNewLevelText() const;
	void SetNewLevelText(const FText& InValue);

	FText GetRewardText() const;
	void SetRewardText(const FText& InValue);

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText NewLevelText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText RewardText;
};
