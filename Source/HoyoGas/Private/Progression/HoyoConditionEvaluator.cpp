#include "Progression/HoyoConditionEvaluator.h"

#include "Progression/HoyoProgressionTags.h"
#include "Progression/HoyoProgressSubsystem.h"

void UHoyoConditionEvaluator::GetDependencyKeys(const FHoyoCheckCondition& Condition, TArray<FName>& OutDependencyKeys) const
{
	(void)Condition;
	(void)OutDependencyKeys;
}

FGameplayTag UHoyoCounterConditionEvaluator::GetEvaluatorTag() const
{
	return HoyoProgressionTags::Check_Counter;
}

bool UHoyoCounterConditionEvaluator::Evaluate(const FHoyoCheckCondition& Condition, const UHoyoProgressSubsystem& ProgressSubsystem) const
{
	FHoyoProgressValue CurrentValue;
	if (!ProgressSubsystem.GetValue(Condition.TargetTag, CurrentValue))
	{
		CurrentValue = FHoyoProgressValue::FromInt(0);
	}

	return CompareValues(CurrentValue, Condition.ExpectedValue, Condition.Operator);
}

void UHoyoCounterConditionEvaluator::GetDependencyKeys(const FHoyoCheckCondition& Condition, TArray<FName>& OutDependencyKeys) const
{
	if (Condition.TargetTag.IsValid())
	{
		OutDependencyKeys.Add(UHoyoProgressSubsystem::MakeValueDependencyKey(Condition.TargetTag));
	}
}

bool UHoyoCounterConditionEvaluator::CompareValues(const FHoyoProgressValue& CurrentValue, const FHoyoProgressValue& ExpectedValue, EHoyoCheckOperator Operator) const
{
	switch (ExpectedValue.ValueType)
	{
	case EHoyoProgressValueType::Int:
		return CompareInt(CurrentValue.ValueType == EHoyoProgressValueType::Float ? FMath::RoundToInt(CurrentValue.FloatValue) : CurrentValue.IntValue, Operator, ExpectedValue.IntValue);
	case EHoyoProgressValueType::Float:
		return CompareFloat(CurrentValue.ValueType == EHoyoProgressValueType::Int ? static_cast<float>(CurrentValue.IntValue) : CurrentValue.FloatValue, Operator, ExpectedValue.FloatValue);
	case EHoyoProgressValueType::Bool:
		return CompareBool(CurrentValue.BoolValue, Operator, ExpectedValue.BoolValue);
	case EHoyoProgressValueType::String:
		return CompareString(CurrentValue.StringValue, Operator, ExpectedValue.StringValue);
	default:
		return false;
	}
}

bool UHoyoCounterConditionEvaluator::CompareInt(int32 CurrentValue, EHoyoCheckOperator Operator, int32 ExpectedValue) const
{
	switch (Operator)
	{
	case EHoyoCheckOperator::Equal:
		return CurrentValue == ExpectedValue;
	case EHoyoCheckOperator::NotEqual:
		return CurrentValue != ExpectedValue;
	case EHoyoCheckOperator::GreaterOrEqual:
		return CurrentValue >= ExpectedValue;
	case EHoyoCheckOperator::Greater:
		return CurrentValue > ExpectedValue;
	case EHoyoCheckOperator::LessOrEqual:
		return CurrentValue <= ExpectedValue;
	case EHoyoCheckOperator::Less:
		return CurrentValue < ExpectedValue;
	default:
		return false;
	}
}

bool UHoyoCounterConditionEvaluator::CompareFloat(float CurrentValue, EHoyoCheckOperator Operator, float ExpectedValue) const
{
	switch (Operator)
	{
	case EHoyoCheckOperator::Equal:
		return FMath::IsNearlyEqual(CurrentValue, ExpectedValue);
	case EHoyoCheckOperator::NotEqual:
		return !FMath::IsNearlyEqual(CurrentValue, ExpectedValue);
	case EHoyoCheckOperator::GreaterOrEqual:
		return CurrentValue >= ExpectedValue || FMath::IsNearlyEqual(CurrentValue, ExpectedValue);
	case EHoyoCheckOperator::Greater:
		return CurrentValue > ExpectedValue && !FMath::IsNearlyEqual(CurrentValue, ExpectedValue);
	case EHoyoCheckOperator::LessOrEqual:
		return CurrentValue <= ExpectedValue || FMath::IsNearlyEqual(CurrentValue, ExpectedValue);
	case EHoyoCheckOperator::Less:
		return CurrentValue < ExpectedValue && !FMath::IsNearlyEqual(CurrentValue, ExpectedValue);
	default:
		return false;
	}
}

bool UHoyoCounterConditionEvaluator::CompareBool(bool bCurrentValue, EHoyoCheckOperator Operator, bool bExpectedValue) const
{
	switch (Operator)
	{
	case EHoyoCheckOperator::Equal:
		return bCurrentValue == bExpectedValue;
	case EHoyoCheckOperator::NotEqual:
		return bCurrentValue != bExpectedValue;
	default:
		return false;
	}
}

bool UHoyoCounterConditionEvaluator::CompareString(const FString& CurrentValue, EHoyoCheckOperator Operator, const FString& ExpectedValue) const
{
	switch (Operator)
	{
	case EHoyoCheckOperator::Equal:
		return CurrentValue == ExpectedValue;
	case EHoyoCheckOperator::NotEqual:
		return CurrentValue != ExpectedValue;
	default:
		return false;
	}
}

FGameplayTag UHoyoAchievementUnlockedConditionEvaluator::GetEvaluatorTag() const
{
	return HoyoProgressionTags::Check_AchievementUnlocked;
}

bool UHoyoAchievementUnlockedConditionEvaluator::Evaluate(const FHoyoCheckCondition& Condition,
	const UHoyoProgressSubsystem& ProgressSubsystem) const
{
	if (Condition.ExpectedValue.ValueType != EHoyoProgressValueType::String || Condition.ExpectedValue.StringValue.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Progression] Check.AchievementUnlocked requires ExpectedValue.StringValue."));
		return false;
	}
	const FName AchievementId(*Condition.ExpectedValue.StringValue);
	const bool bUnlocked = ProgressSubsystem.IsAchievementUnlocked(AchievementId);
	switch (Condition.Operator)
	{
		case EHoyoCheckOperator::Equal:
		case EHoyoCheckOperator::GreaterOrEqual:
		case EHoyoCheckOperator::Greater:
			return bUnlocked;
		
		case EHoyoCheckOperator::NotEqual:
		case EHoyoCheckOperator::LessOrEqual:
		case EHoyoCheckOperator::Less:
			return !bUnlocked;
		
		default:
			return false;
	}
}

void UHoyoAchievementUnlockedConditionEvaluator::GetDependencyKeys(const FHoyoCheckCondition& Condition, TArray<FName>& OutDependencyKeys) const
{
	if (Condition.ExpectedValue.ValueType == EHoyoProgressValueType::String && !Condition.ExpectedValue.StringValue.IsEmpty())
	{
		OutDependencyKeys.Add(UHoyoProgressSubsystem::MakeAchievementDependencyKey(FName(*Condition.ExpectedValue.StringValue)));
	}
}
