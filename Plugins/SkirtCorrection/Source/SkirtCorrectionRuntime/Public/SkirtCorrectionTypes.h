#pragma once

#include "CoreMinimal.h"
#include "SkirtCorrectionTypes.generated.h"

UENUM(BlueprintType)
enum class ESkirtDriverRotationChannel : uint8
{
    Pitch UMETA(DisplayName = "Pitch (Y)"),
    Yaw UMETA(DisplayName = "Yaw (Z)"),
    Roll UMETA(DisplayName = "Roll (X)")
};

UENUM(BlueprintType)
enum class ESkirtDriverAngleMode : uint8
{
    ChannelDelta UMETA(DisplayName = "Channel Delta"),
    TwistAngle UMETA(DisplayName = "Twist Around Axis")
};
