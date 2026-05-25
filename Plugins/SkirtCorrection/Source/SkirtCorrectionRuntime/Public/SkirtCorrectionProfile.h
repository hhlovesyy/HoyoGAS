#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SkirtCorrectionTypes.h"
#include "SkirtCorrectionProfile.generated.h"

class UCurveFloat;

USTRUCT(BlueprintType)
struct SKIRTCORRECTIONRUNTIME_API FSkirtCorrectionProfileBoneRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Rule")
    FName BoneName = NAME_None;

    UPROPERTY(EditAnywhere, Category = "Rule")
    FRotator BaseRotationOffset = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, Category = "Rule")
    FRotator AdditiveRotationAtFullAlpha = FRotator::ZeroRotator;

    // Optional per-bone remap. Input is side alpha in 0..1.
    UPROPERTY(EditAnywhere, Category = "Rule")
    TObjectPtr<UCurveFloat> InfluenceCurve = nullptr;
};

USTRUCT(BlueprintType)
struct SKIRTCORRECTIONRUNTIME_API FSkirtCorrectionProfileSide
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Driver")
    float StartDeg = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Driver")
    float FullDeg = 35.0f;

    // Optional side-level response curve. Input is the linear remapped driver alpha in 0..1.
    UPROPERTY(EditAnywhere, Category = "Driver")
    TObjectPtr<UCurveFloat> DriverCurve = nullptr;

    UPROPERTY(EditAnywhere, Category = "Skirt")
    TArray<FSkirtCorrectionProfileBoneRule> Rules;
};

UCLASS(BlueprintType)
class SKIRTCORRECTIONRUNTIME_API USkirtCorrectionProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Driver")
    FName PelvisBoneName = TEXT("Hips");

    UPROPERTY(EditAnywhere, Category = "Driver")
    FName LeftThighBoneName = TEXT("Left-leg");

    UPROPERTY(EditAnywhere, Category = "Driver")
    FName RightThighBoneName = TEXT("Right-leg");

    UPROPERTY(EditAnywhere, Category = "Driver")
    ESkirtDriverAngleMode DriverAngleMode = ESkirtDriverAngleMode::ChannelDelta;

    UPROPERTY(EditAnywhere, Category = "Driver")
    ESkirtDriverRotationChannel DriverChannel = ESkirtDriverRotationChannel::Roll;

    UPROPERTY(EditAnywhere, Category = "Driver")
    bool bUseAbsoluteDriverAngle = true;

    UPROPERTY(EditAnywhere, Category = "Left")
    FSkirtCorrectionProfileSide LeftSide;

    UPROPERTY(EditAnywhere, Category = "Right")
    FSkirtCorrectionProfileSide RightSide;
};
