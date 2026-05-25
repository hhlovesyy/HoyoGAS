#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Progression/HoyoCheckTypes.h"
#include "UObject/Object.h"
#include "HoyoConditionEvaluator.generated.h"

class UHoyoProgressSubsystem;

UCLASS(Abstract, Blueprintable)
class HOYOGAS_API UHoyoConditionEvaluator : public UObject
{
	GENERATED_BODY()

public:
	virtual FGameplayTag GetEvaluatorTag() const PURE_VIRTUAL(UHoyoConditionEvaluator::GetEvaluatorTag, return FGameplayTag();)
	virtual bool Evaluate(const FHoyoCheckCondition& Condition, const UHoyoProgressSubsystem& ProgressSubsystem) const PURE_VIRTUAL(UHoyoConditionEvaluator::Evaluate, return false;)
	virtual void GetDependencyKeys(const FHoyoCheckCondition& Condition, TArray<FName>& OutDependencyKeys) const;
};

UCLASS()
class HOYOGAS_API UHoyoCounterConditionEvaluator : public UHoyoConditionEvaluator
{
	GENERATED_BODY()

public:
	virtual FGameplayTag GetEvaluatorTag() const override;
	virtual bool Evaluate(const FHoyoCheckCondition& Condition, const UHoyoProgressSubsystem& ProgressSubsystem) const override;
	virtual void GetDependencyKeys(const FHoyoCheckCondition& Condition, TArray<FName>& OutDependencyKeys) const override;

private:
	bool CompareValues(const FHoyoProgressValue& CurrentValue, const FHoyoProgressValue& ExpectedValue, EHoyoCheckOperator Operator) const;
	bool CompareInt(int32 CurrentValue, EHoyoCheckOperator Operator, int32 ExpectedValue) const;
	bool CompareFloat(float CurrentValue, EHoyoCheckOperator Operator, float ExpectedValue) const;
	bool CompareBool(bool bCurrentValue, EHoyoCheckOperator Operator, bool bExpectedValue) const;
	bool CompareString(const FString& CurrentValue, EHoyoCheckOperator Operator, const FString& ExpectedValue) const;
};

UCLASS()
class HOYOGAS_API UHoyoAchievementUnlockedConditionEvaluator:public UHoyoConditionEvaluator
{
	GENERATED_BODY()
	
public:
	virtual FGameplayTag GetEvaluatorTag() const override;
	virtual bool Evaluate(const FHoyoCheckCondition& Condition, const UHoyoProgressSubsystem& ProgressSubsystem) const override;
	virtual void GetDependencyKeys(const FHoyoCheckCondition& Condition, TArray<FName>& OutDependencyKeys) const override;
};
