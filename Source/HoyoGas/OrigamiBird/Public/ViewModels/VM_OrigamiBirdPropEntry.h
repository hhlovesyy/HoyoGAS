#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdMatchTypes.h"
#include "MVVMViewModelBase.h"
#include "VM_OrigamiBirdPropEntry.generated.h"

class UTexture2D;

// 道具 ListView 的单行 ViewModel。
// 它只负责把“道具定义 + 当前持有数量”整理成 UI 能直接显示的字段。
UCLASS(BlueprintType)
class HOYOGAS_API UVM_OrigamiBirdPropEntry : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void InitializeFromDefinition(FName InPropId, const FOrigamiBirdPropDefinitionRow& Definition, int32 InCount);

	FName GetPropId() const;
	void SetPropId(FName InValue);

	FText GetDisplayNameText() const;
	void SetDisplayNameText(const FText& InValue);

	FText GetDescriptionText() const;
	void SetDescriptionText(const FText& InValue);

	FText GetCountText() const;
	void SetCountText(const FText& InValue);

	FText GetStackRuleText() const;
	void SetStackRuleText(const FText& InValue);

	FText GetUsageText() const;
	void SetUsageText(const FText& InValue);

	bool GetIsStackable() const;
	void SetIsStackable(bool bInValue);

	EOrigamiBirdPropTargetType GetTargetType() const;
	void SetTargetType(EOrigamiBirdPropTargetType InValue);

	int32 GetCount() const;
	void SetCount(int32 InValue);

	UTexture2D* GetIconTexture() const;

private:
	void SetIconTexture(UTexture2D* InTexture);

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FName PropId = NAME_None;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText DisplayNameText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText DescriptionText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText CountText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText StackRuleText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText UsageText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter = "GetIsStackable", Setter = "SetIsStackable", meta = (AllowPrivateAccess = "true"))
	bool bIsStackable = true;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	EOrigamiBirdPropTargetType TargetType = EOrigamiBirdPropTargetType::None;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	int32 Count = 0;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> IconTexture;
};
