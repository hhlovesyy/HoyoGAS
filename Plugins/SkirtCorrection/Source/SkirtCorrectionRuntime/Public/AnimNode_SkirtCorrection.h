#pragma once

#include "CoreMinimal.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "SkirtCorrectionTypes.h"
#include "AnimNode_SkirtCorrection.generated.h"

class UCurveFloat;
class USkirtCorrectionProfile;

USTRUCT(BlueprintType)
struct SKIRTCORRECTIONRUNTIME_API FSkirtCorrectionBoneRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Rule")
    FBoneReference Bone;

    // Static correction applied even when driver alpha is zero.
    UPROPERTY(EditAnywhere, Category = "Rule")
    FRotator BaseRotationOffset = FRotator::ZeroRotator;

    // Extra bone-space rotation when this side reaches full driver alpha.
    UPROPERTY(EditAnywhere, Category = "Rule")
    FRotator AdditiveRotationAtFullAlpha = FRotator::ZeroRotator;

    // Optional per-bone remap. Input is side alpha in 0..1.
    UPROPERTY(EditAnywhere, Category = "Rule")
    TObjectPtr<UCurveFloat> InfluenceCurve = nullptr;
};

USTRUCT(BlueprintInternalUseOnly)
struct SKIRTCORRECTIONRUNTIME_API FAnimNode_SkirtCorrection : public FAnimNode_SkeletalControlBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Settings", meta = (PinShownByDefault))
    bool bEnable = true;

    // Optional reusable profile asset. When assigned, runtime settings are read from it.
    UPROPERTY(EditAnywhere, Category = "Settings")
    TObjectPtr<USkirtCorrectionProfile> Profile = nullptr;

    UPROPERTY(EditAnywhere, Category = "Driver")
    FBoneReference PelvisBone;

    UPROPERTY(EditAnywhere, Category = "Driver")
    FBoneReference LeftThighBone;

    UPROPERTY(EditAnywhere, Category = "Driver")
    FBoneReference RightThighBone;

    UPROPERTY(EditAnywhere, Category = "Driver")
    ESkirtDriverAngleMode DriverAngleMode = ESkirtDriverAngleMode::ChannelDelta;

    UPROPERTY(EditAnywhere, Category = "Driver")
    ESkirtDriverRotationChannel DriverChannel = ESkirtDriverRotationChannel::Roll;

    UPROPERTY(EditAnywhere, Category = "Driver")
    bool bUseAbsoluteDriverAngle = true;

    UPROPERTY(EditAnywhere, Category = "Driver")
    float LeftStartDeg = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Driver")
    float LeftFullDeg = 35.0f;

    UPROPERTY(EditAnywhere, Category = "Driver")
    TObjectPtr<UCurveFloat> LeftDriverCurve = nullptr;

    UPROPERTY(EditAnywhere, Category = "Driver")
    float RightStartDeg = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Driver")
    float RightFullDeg = 35.0f;

    UPROPERTY(EditAnywhere, Category = "Driver")
    TObjectPtr<UCurveFloat> RightDriverCurve = nullptr;

    UPROPERTY(EditAnywhere, Category = "Skirt")
    TArray<FSkirtCorrectionBoneRule> LeftSkirtRules;

    UPROPERTY(EditAnywhere, Category = "Skirt")
    TArray<FSkirtCorrectionBoneRule> RightSkirtRules;

public:
    FAnimNode_SkirtCorrection();

    virtual void EvaluateSkeletalControl_AnyThread(
        FComponentSpacePoseContext& Output,
        TArray<FBoneTransform>& OutBoneTransforms) override;

    virtual bool IsValidToEvaluate(
        const USkeleton* Skeleton,
        const FBoneContainer& RequiredBones) override;

    virtual void InitializeBoneReferences(
        const FBoneContainer& RequiredBones) override;

private:
    struct FCachedBoneRule
    {
        FBoneReference Bone;
        FRotator BaseRotationOffset = FRotator::ZeroRotator;
        FRotator AdditiveRotationAtFullAlpha = FRotator::ZeroRotator;
        const UCurveFloat* InfluenceCurve = nullptr;
    };

    static float ExtractChannelDeg(const FRotator& Rot, ESkirtDriverRotationChannel Channel);
    static FVector GetAxisVector(ESkirtDriverRotationChannel Axis);
    static float MapAngleToAlpha(float CurrentDeg, float StartDeg, float FullDeg);
    static float ExtractTwistAngleDeg(const FQuat& DeltaQuat, const FVector& AxisCS);
    static float EvaluateCurve01(const UCurveFloat* Curve, float InputAlpha);

    void BuildCachedSettings(const FBoneContainer& RequiredBones);

    float ComputeDriverAngleDeg(
        const FQuat& InitialRelQuat,
        const FQuat& CurrentRelQuat) const;

private:
    bool bHasCalibration = false;

    FBoneReference RuntimePelvisBone;
    FBoneReference RuntimeLeftThighBone;
    FBoneReference RuntimeRightThighBone;

    ESkirtDriverAngleMode RuntimeDriverAngleMode = ESkirtDriverAngleMode::ChannelDelta;
    ESkirtDriverRotationChannel RuntimeDriverChannel = ESkirtDriverRotationChannel::Roll;
    bool bRuntimeUseAbsoluteDriverAngle = true;

    float RuntimeLeftStartDeg = 5.0f;
    float RuntimeLeftFullDeg = 35.0f;
    float RuntimeRightStartDeg = 5.0f;
    float RuntimeRightFullDeg = 35.0f;
    const UCurveFloat* RuntimeLeftDriverCurve = nullptr;
    const UCurveFloat* RuntimeRightDriverCurve = nullptr;

    TArray<FCachedBoneRule> RuntimeLeftRules;
    TArray<FCachedBoneRule> RuntimeRightRules;

    FQuat InitialLeftThighRelQuat = FQuat::Identity;
    FQuat InitialRightThighRelQuat = FQuat::Identity;
};
