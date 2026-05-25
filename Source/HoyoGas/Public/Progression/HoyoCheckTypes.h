#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HoyoCheckTypes.generated.h"

UENUM(BlueprintType)
enum class EHoyoCheckOperator : uint8
{
	Equal,
	NotEqual,
	GreaterOrEqual,
	Greater,
	LessOrEqual,
	Less
};

UENUM(BlueprintType)
enum class EHoyoProgressValueType : uint8
{
	Int,
	Float,
	Bool,
	String
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoProgressValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	EHoyoProgressValueType ValueType = EHoyoProgressValueType::Int;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	float FloatValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool BoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FString StringValue;

	static FHoyoProgressValue FromInt(int32 InValue)
	{
		FHoyoProgressValue Value;
		Value.ValueType = EHoyoProgressValueType::Int;
		Value.IntValue = InValue;
		return Value;
	}

	static FHoyoProgressValue FromFloat(float InValue)
	{
		FHoyoProgressValue Value;
		Value.ValueType = EHoyoProgressValueType::Float;
		Value.FloatValue = InValue;
		return Value;
	}

	static FHoyoProgressValue FromBool(bool bInValue)
	{
		FHoyoProgressValue Value;
		Value.ValueType = EHoyoProgressValueType::Bool;
		Value.BoolValue = bInValue;
		return Value;
	}

	static FHoyoProgressValue FromString(const FString& InValue)
	{
		FHoyoProgressValue Value;
		Value.ValueType = EHoyoProgressValueType::String;
		Value.StringValue = InValue;
		return Value;
	}
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoCheckCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Check")
	FGameplayTag CheckTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Check")
	FGameplayTag TargetTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Check")
	EHoyoCheckOperator Operator = EHoyoCheckOperator::GreaterOrEqual;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Check")
	FHoyoProgressValue ExpectedValue;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoCheckConditionSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Check")
	bool bMatchAll = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Check")
	TArray<FHoyoCheckCondition> Conditions;
};
