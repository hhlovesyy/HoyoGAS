#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "SurvivorPlayerCameraManager.generated.h"

class ASurvivorCharacter;
class UCameraShakeBase;
//UE 里 PlayerController 会持有一个 PlayerCameraManager，每帧问它：这一帧我的相机位置、旋转、FOV、后处理应该是什么？
//答案就写进 FTViewTarget& OutVT 里面，尤其是 OutVT.POV。你可以把 POV 理解成“这一帧最终相机结果”。

UCLASS()
class HOYOGAS_API ASurvivorPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ASurvivorPlayerCameraManager();

	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "SurvivorArena|Camera")
	UCameraShakeBase* PlayFeedbackCameraShake(
		TSubclassOf<UCameraShakeBase> CameraShakeClass,
		float Scale = 1.0f,
		ECameraShakePlaySpace PlaySpace = ECameraShakePlaySpace::CameraLocal,
		FRotator UserPlaySpaceRot = FRotator::ZeroRotator);

protected:
	void UpdateSurvivorViewTarget(FTViewTarget& OutVT, float DeltaTime);
	void ApplyFeedbackShakeFallback(float DeltaTime, FMinimalViewInfo& InOutPOV);
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Camera|Feedback", meta = (ClampMin = "0.0"))
	float FeedbackShakeFallbackDuration = 0.18f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Camera|Feedback", meta = (ClampMin = "0.0"))
	float FeedbackShakeFallbackLocationAmplitude = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Camera|Feedback", meta = (ClampMin = "0.0"))
	float FeedbackShakeFallbackRotationAmplitude = 0.75f;

private:
	bool bHasSmoothedFocusLocation = false;
	FVector SmoothedFocusLocation = FVector::ZeroVector;
	float FeedbackShakeTimeRemaining = 0.0f;
	float FeedbackShakeDurationRemaining = 0.0f;
	float FeedbackShakeScale = 0.0f;
	float FeedbackShakeLocationPhase = 0.0f;
	float FeedbackShakeRotationPhase = 0.0f;
};
