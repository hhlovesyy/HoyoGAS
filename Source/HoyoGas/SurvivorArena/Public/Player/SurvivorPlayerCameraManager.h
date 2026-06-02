#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "SurvivorPlayerCameraManager.generated.h"

class ASurvivorCharacter;

UCLASS()
class HOYOGAS_API ASurvivorPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ASurvivorPlayerCameraManager();

	//核心心跳函数：UpdateViewTarget。这是这个类最重要的函数，引擎每一帧都会调用它来决定最终画面的渲染视角。它的逻辑按顺序分为三步：
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

protected:
	FVector ResolveCameraFocusLocation(const AActor* ViewTarget) const;
	const ASurvivorCharacter* ResolveSurvivorCharacter(const AActor* ViewTarget) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Camera")
	FRotator CameraRotation = FRotator(-60.0f, 0.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Camera", meta = (ClampMin = "100.0"))
	float CameraDistance = 1400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Camera")
	FVector CameraFocusOffset = FVector(0.0f, 0.0f, 100.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Camera", meta = (ClampMin = "0.0"))
	float CameraFollowLagSpeed = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Camera", meta = (ClampMin = "1.0"))
	float PerspectiveFOV = 50.0f;

private:
	bool bHasSmoothedFocusLocation = false;
	FVector SmoothedFocusLocation = FVector::ZeroVector;
};
