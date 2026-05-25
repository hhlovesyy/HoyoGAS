#include "AnimNode_SkirtCorrection.h"

#include "SkirtCorrectionProfile.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimationRuntime.h"
#include "Curves/CurveFloat.h"

namespace
{
    static void InitializeRuntimeBone(
        FBoneReference& OutBone,
        const FName BoneName,
        const FBoneContainer& RequiredBones)
    {
        OutBone.BoneName = BoneName;
        OutBone.Initialize(RequiredBones);
    }
}

FAnimNode_SkirtCorrection::FAnimNode_SkirtCorrection()
{
    PelvisBone.BoneName = TEXT("pelvis");
    LeftThighBone.BoneName = TEXT("thigh_l");
    RightThighBone.BoneName = TEXT("thigh_r");
}

float FAnimNode_SkirtCorrection::ExtractChannelDeg(
    const FRotator& Rot,
    ESkirtDriverRotationChannel Channel)
{
    switch (Channel)
    {
    case ESkirtDriverRotationChannel::Pitch:
        return Rot.Pitch;
    case ESkirtDriverRotationChannel::Yaw:
        return Rot.Yaw;
    case ESkirtDriverRotationChannel::Roll:
    default:
        return Rot.Roll;
    }
}

FVector FAnimNode_SkirtCorrection::GetAxisVector(ESkirtDriverRotationChannel Axis)
{
    switch (Axis)
    {
    case ESkirtDriverRotationChannel::Pitch:
        return FVector::RightVector;
    case ESkirtDriverRotationChannel::Yaw:
        return FVector::UpVector;
    case ESkirtDriverRotationChannel::Roll:
    default:
        return FVector::ForwardVector;
    }
}

float FAnimNode_SkirtCorrection::MapAngleToAlpha(
    float CurrentDeg,
    float StartDeg,
    float FullDeg)
{
    if (FMath::IsNearlyEqual(StartDeg, FullDeg))
    {
        return 0.0f;
    }

    const float T = (CurrentDeg - StartDeg) / (FullDeg - StartDeg);
    return FMath::Clamp(T, 0.0f, 1.0f);
}

float FAnimNode_SkirtCorrection::ExtractTwistAngleDeg(
    const FQuat& InDeltaQuat,
    const FVector& AxisCS)
{
    const FVector NormalizedAxis = AxisCS.GetSafeNormal();
    if (NormalizedAxis.IsNearlyZero())
    {
        return 0.0f;
    }

    FQuat DeltaQuat = InDeltaQuat.GetNormalized();
    if (DeltaQuat.W < 0.0f)
    {
        DeltaQuat.X *= -1.0f;
        DeltaQuat.Y *= -1.0f;
        DeltaQuat.Z *= -1.0f;
        DeltaQuat.W *= -1.0f;
    }

    const FVector DeltaVector(DeltaQuat.X, DeltaQuat.Y, DeltaQuat.Z);
    const FVector ProjectedVector = FVector::DotProduct(DeltaVector, NormalizedAxis) * NormalizedAxis;

    FQuat TwistQuat(ProjectedVector.X, ProjectedVector.Y, ProjectedVector.Z, DeltaQuat.W);
    if (TwistQuat.SizeSquared() <= SMALL_NUMBER)
    {
        return 0.0f;
    }

    TwistQuat.Normalize();

    FVector TwistAxis = NormalizedAxis;
    float TwistAngleRad = 0.0f;
    TwistQuat.ToAxisAndAngle(TwistAxis, TwistAngleRad);

    if (TwistAngleRad > PI)
    {
        TwistAngleRad -= 2.0f * PI;
    }

    const float Sign = FVector::DotProduct(TwistAxis, NormalizedAxis) >= 0.0f ? 1.0f : -1.0f;
    return FRotator::NormalizeAxis(FMath::RadiansToDegrees(TwistAngleRad) * Sign);
}

float FAnimNode_SkirtCorrection::EvaluateCurve01(const UCurveFloat* Curve, float InputAlpha)
{
    const float ClampedInput = FMath::Clamp(InputAlpha, 0.0f, 1.0f);
    if (!Curve)
    {
        return ClampedInput;
    }

    return FMath::Clamp(Curve->GetFloatValue(ClampedInput), 0.0f, 1.0f);
}

void FAnimNode_SkirtCorrection::BuildCachedSettings(const FBoneContainer& RequiredBones)
{
    RuntimeLeftRules.Reset();
    RuntimeRightRules.Reset();

    if (Profile)
    {
        RuntimeDriverAngleMode = Profile->DriverAngleMode;
        RuntimeDriverChannel = Profile->DriverChannel;
        bRuntimeUseAbsoluteDriverAngle = Profile->bUseAbsoluteDriverAngle;

        RuntimeLeftStartDeg = Profile->LeftSide.StartDeg;
        RuntimeLeftFullDeg = Profile->LeftSide.FullDeg;
        RuntimeRightStartDeg = Profile->RightSide.StartDeg;
        RuntimeRightFullDeg = Profile->RightSide.FullDeg;
        RuntimeLeftDriverCurve = Profile->LeftSide.DriverCurve;
        RuntimeRightDriverCurve = Profile->RightSide.DriverCurve;

        InitializeRuntimeBone(RuntimePelvisBone, Profile->PelvisBoneName, RequiredBones);
        InitializeRuntimeBone(RuntimeLeftThighBone, Profile->LeftThighBoneName, RequiredBones);
        InitializeRuntimeBone(RuntimeRightThighBone, Profile->RightThighBoneName, RequiredBones);

        auto CopyProfileRules =
            [&](const TArray<FSkirtCorrectionProfileBoneRule>& SourceRules, TArray<FCachedBoneRule>& OutRules)
        {
            OutRules.Reserve(SourceRules.Num());

            for (const FSkirtCorrectionProfileBoneRule& SourceRule : SourceRules)
            {
                FCachedBoneRule CachedRule;
                InitializeRuntimeBone(CachedRule.Bone, SourceRule.BoneName, RequiredBones);
                CachedRule.BaseRotationOffset = SourceRule.BaseRotationOffset;
                CachedRule.AdditiveRotationAtFullAlpha = SourceRule.AdditiveRotationAtFullAlpha;
                CachedRule.InfluenceCurve = SourceRule.InfluenceCurve;
                OutRules.Add(CachedRule);
            }
        };

        CopyProfileRules(Profile->LeftSide.Rules, RuntimeLeftRules);
        CopyProfileRules(Profile->RightSide.Rules, RuntimeRightRules);
    }
    else
    {
        RuntimeDriverAngleMode = DriverAngleMode;
        RuntimeDriverChannel = DriverChannel;
        bRuntimeUseAbsoluteDriverAngle = bUseAbsoluteDriverAngle;

        RuntimeLeftStartDeg = LeftStartDeg;
        RuntimeLeftFullDeg = LeftFullDeg;
        RuntimeRightStartDeg = RightStartDeg;
        RuntimeRightFullDeg = RightFullDeg;
        RuntimeLeftDriverCurve = LeftDriverCurve;
        RuntimeRightDriverCurve = RightDriverCurve;

        RuntimePelvisBone = PelvisBone;
        RuntimeLeftThighBone = LeftThighBone;
        RuntimeRightThighBone = RightThighBone;

        RuntimePelvisBone.Initialize(RequiredBones);
        RuntimeLeftThighBone.Initialize(RequiredBones);
        RuntimeRightThighBone.Initialize(RequiredBones);

        auto CopyInlineRules =
            [&](const TArray<FSkirtCorrectionBoneRule>& SourceRules, TArray<FCachedBoneRule>& OutRules)
        {
            OutRules.Reserve(SourceRules.Num());

            for (const FSkirtCorrectionBoneRule& SourceRule : SourceRules)
            {
                FCachedBoneRule CachedRule;
                CachedRule.Bone = SourceRule.Bone;
                CachedRule.Bone.Initialize(RequiredBones);
                CachedRule.BaseRotationOffset = SourceRule.BaseRotationOffset;
                CachedRule.AdditiveRotationAtFullAlpha = SourceRule.AdditiveRotationAtFullAlpha;
                CachedRule.InfluenceCurve = SourceRule.InfluenceCurve;
                OutRules.Add(CachedRule);
            }
        };

        CopyInlineRules(LeftSkirtRules, RuntimeLeftRules);
        CopyInlineRules(RightSkirtRules, RuntimeRightRules);
    }

    bHasCalibration = false;
    InitialLeftThighRelQuat = FQuat::Identity;
    InitialRightThighRelQuat = FQuat::Identity;
}

float FAnimNode_SkirtCorrection::ComputeDriverAngleDeg(
    const FQuat& InitialRelQuat,
    const FQuat& CurrentRelQuat) const
{
    float DriverAngleDeg = 0.0f;

    if (RuntimeDriverAngleMode == ESkirtDriverAngleMode::ChannelDelta)
    {
        const FRotator InitialRot = InitialRelQuat.Rotator();
        const FRotator CurrentRot = CurrentRelQuat.Rotator();

        const float InitialChannelDeg = ExtractChannelDeg(InitialRot, RuntimeDriverChannel);
        const float CurrentChannelDeg = ExtractChannelDeg(CurrentRot, RuntimeDriverChannel);
        DriverAngleDeg = FMath::FindDeltaAngleDegrees(InitialChannelDeg, CurrentChannelDeg);
    }
    else
    {
        FQuat DeltaQuat = CurrentRelQuat * InitialRelQuat.Inverse();
        DeltaQuat.Normalize();

        const FVector DriverAxisCS = InitialRelQuat.RotateVector(GetAxisVector(RuntimeDriverChannel));
        DriverAngleDeg = ExtractTwistAngleDeg(DeltaQuat, DriverAxisCS);
    }

    if (bRuntimeUseAbsoluteDriverAngle)
    {
        DriverAngleDeg = FMath::Abs(DriverAngleDeg);
    }

    return DriverAngleDeg;
}

bool FAnimNode_SkirtCorrection::IsValidToEvaluate(
    const USkeleton* Skeleton,
    const FBoneContainer& RequiredBones)
{
    return bEnable
        && RuntimePelvisBone.IsValidToEvaluate(RequiredBones)
        && RuntimeLeftThighBone.IsValidToEvaluate(RequiredBones)
        && RuntimeRightThighBone.IsValidToEvaluate(RequiredBones);
}

void FAnimNode_SkirtCorrection::InitializeBoneReferences(
    const FBoneContainer& RequiredBones)
{
    BuildCachedSettings(RequiredBones);
}

void FAnimNode_SkirtCorrection::EvaluateSkeletalControl_AnyThread(
    FComponentSpacePoseContext& Output,
    TArray<FBoneTransform>& OutBoneTransforms)
{
    OutBoneTransforms.Reset();

    const float NodeAlpha = FMath::Clamp(ActualAlpha, 0.0f, 1.0f);
    if (!bEnable || NodeAlpha <= 0.0f)
    {
        return;
    }

    const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();

    const FCompactPoseBoneIndex PelvisIndex = RuntimePelvisBone.GetCompactPoseIndex(BoneContainer);
    const FCompactPoseBoneIndex LeftThighIndex = RuntimeLeftThighBone.GetCompactPoseIndex(BoneContainer);
    const FCompactPoseBoneIndex RightThighIndex = RuntimeRightThighBone.GetCompactPoseIndex(BoneContainer);

    const FTransform PelvisCS = Output.Pose.GetComponentSpaceTransform(PelvisIndex);
    const FTransform LeftThighCS = Output.Pose.GetComponentSpaceTransform(LeftThighIndex);
    const FTransform RightThighCS = Output.Pose.GetComponentSpaceTransform(RightThighIndex);

    const FQuat CurrentLeftRelQuat =
        LeftThighCS.GetRelativeTransform(PelvisCS).GetRotation().GetNormalized();
    const FQuat CurrentRightRelQuat =
        RightThighCS.GetRelativeTransform(PelvisCS).GetRotation().GetNormalized();

    if (!bHasCalibration)
    {
        InitialLeftThighRelQuat = CurrentLeftRelQuat;
        InitialRightThighRelQuat = CurrentRightRelQuat;
        bHasCalibration = true;
        return;
    }

    const float LeftAngleDeg = ComputeDriverAngleDeg(InitialLeftThighRelQuat, CurrentLeftRelQuat);
    const float RightAngleDeg = ComputeDriverAngleDeg(InitialRightThighRelQuat, CurrentRightRelQuat);

    const float LeftDriverAlpha = EvaluateCurve01(
        RuntimeLeftDriverCurve,
        MapAngleToAlpha(LeftAngleDeg, RuntimeLeftStartDeg, RuntimeLeftFullDeg));

    const float RightDriverAlpha = EvaluateCurve01(
        RuntimeRightDriverCurve,
        MapAngleToAlpha(RightAngleDeg, RuntimeRightStartDeg, RuntimeRightFullDeg));

    const FTransform ComponentTransform = Output.AnimInstanceProxy->GetComponentTransform();

    auto ApplyRules = [&](const TArray<FCachedBoneRule>& Rules, float SideDriverAlpha)
    {
        for (const FCachedBoneRule& Rule : Rules)
        {
            if (!Rule.Bone.IsValidToEvaluate(BoneContainer))
            {
                continue;
            }

            const float RuleDriverAlpha = EvaluateCurve01(Rule.InfluenceCurve, SideDriverAlpha);
            const FRotator FinalAddRot(
                (Rule.BaseRotationOffset.Pitch + Rule.AdditiveRotationAtFullAlpha.Pitch * RuleDriverAlpha) * NodeAlpha,
                (Rule.BaseRotationOffset.Yaw + Rule.AdditiveRotationAtFullAlpha.Yaw * RuleDriverAlpha) * NodeAlpha,
                (Rule.BaseRotationOffset.Roll + Rule.AdditiveRotationAtFullAlpha.Roll * RuleDriverAlpha) * NodeAlpha);

            if (FinalAddRot.IsNearlyZero())
            {
                continue;
            }

            const FCompactPoseBoneIndex BoneIndex = Rule.Bone.GetCompactPoseIndex(BoneContainer);
            FTransform BoneCS = Output.Pose.GetComponentSpaceTransform(BoneIndex);

            FAnimationRuntime::ConvertCSTransformToBoneSpace(
                ComponentTransform,
                Output.Pose,
                BoneCS,
                BoneIndex,
                BCS_BoneSpace);

            BoneCS.SetRotation((FinalAddRot.Quaternion() * BoneCS.GetRotation()).GetNormalized());

            FAnimationRuntime::ConvertBoneSpaceTransformToCS(
                ComponentTransform,
                Output.Pose,
                BoneCS,
                BoneIndex,
                BCS_BoneSpace);

            OutBoneTransforms.Add(FBoneTransform(BoneIndex, BoneCS));
        }
    };

    ApplyRules(RuntimeLeftRules, LeftDriverAlpha);
    ApplyRules(RuntimeRightRules, RightDriverAlpha);

    OutBoneTransforms.Sort(FCompareBoneTransformIndex());
}
