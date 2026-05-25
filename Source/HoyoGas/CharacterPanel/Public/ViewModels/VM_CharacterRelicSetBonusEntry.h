#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VM_CharacterRelicSetBonusEntry.generated.h"

// 遗器套装激活列表项 VM：例如“快枪手 4 件，已激活 2/4 件套”。
UCLASS(BlueprintType)
class HOYOGAS_API UVM_CharacterRelicSetBonusEntry : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	FName GetSetId() const;
	void SetSetId(FName InValue);

	FText GetSetNameText() const;
	void SetSetNameText(const FText& InValue);

	FText GetActivationText() const;
	void SetActivationText(const FText& InValue);

protected:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FName SetId = NAME_None;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText SetNameText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText ActivationText;
};
