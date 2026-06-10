#include "Player/SurvivorPlayerCameraManager.h"

#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Core/SurvivorArenaLog.h"
#include "GameFramework/PlayerController.h"
#include "Player/SurvivorCharacter.h"

/*
*接入 Survivor 相机反馈震屏

- 给 SurvivorPlayerCameraManager 暴露蓝图可调用的 PlayFeedbackCameraShake
- 修正自定义俯视相机下 CameraModifier/CameraShake 的应用顺序
- 增加 fallback 震屏，保证 GameplayCue 触发后能看到镜头反馈
- 为高松灯 PoemBurst 等卡牌技能效果预留镜头反馈入口
 */
ASurvivorPlayerCameraManager::ASurvivorPlayerCameraManager()
{
	ViewPitchMin = -89.0f;
	ViewPitchMax = 0.0f;
	DefaultFOV = PerspectiveFOV;
}

UCameraShakeBase* ASurvivorPlayerCameraManager::PlayFeedbackCameraShake(
	TSubclassOf<UCameraShakeBase> CameraShakeClass,
	float Scale,
	ECameraShakePlaySpace PlaySpace,
	FRotator UserPlaySpaceRot)
{
	UE_LOG(LogSurvivorArena, Log, TEXT("PlayFeedbackCameraShake requested. CameraManager=%s CameraShakeClass=%s Scale=%.2f"),
		*GetNameSafe(this),
		*GetNameSafe(CameraShakeClass),
		Scale);

	if (!CameraShakeClass)
	{
		UE_LOG(LogSurvivorArena, Verbose, TEXT("PlayFeedbackCameraShake skipped because CameraShakeClass is null. CameraManager=%s"), *GetNameSafe(this));
		return nullptr;
	}

	UCameraShakeBase* ShakeInstance = StartCameraShake(CameraShakeClass, Scale, PlaySpace, UserPlaySpaceRot);
	UE_LOG(LogSurvivorArena, Log, TEXT("PlayFeedbackCameraShake StartCameraShake finished. CameraManager=%s CameraShakeClass=%s Scale=%.2f bShakeValid=%s"),
		*GetNameSafe(this),
		*GetNameSafe(CameraShakeClass),
		Scale,
		ShakeInstance ? TEXT("true") : TEXT("false"));

	if (Scale > 0.0f && FeedbackShakeFallbackDuration > 0.0f)
	{
		FeedbackShakeScale = Scale;
		FeedbackShakeTimeRemaining = FeedbackShakeFallbackDuration;
		FeedbackShakeDurationRemaining = FeedbackShakeFallbackDuration;
		FeedbackShakeLocationPhase = FMath::FRandRange(0.0f, 2.0f * PI);
		FeedbackShakeRotationPhase = FMath::FRandRange(0.0f, 2.0f * PI);
	}
	else
	{
		FeedbackShakeScale = 0.0f;
		FeedbackShakeTimeRemaining = 0.0f;
		FeedbackShakeDurationRemaining = 0.0f;
	}

	return ShakeInstance;
}

void ASurvivorPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	//每一帧，UE 都会问 PlayerCameraManager：这一帧玩家最终应该从哪里看、朝哪里看、FOV 是多少？这个函数就是回答这个问题的地方。
	//OutVT 里的 POV 就是最终相机数据：
	//每帧执行顺序如下：游戏 Tick->PlayerController 更新相机->PlayerCameraManager 更新相机->ASurvivorPlayerCameraManager::UpdateViewTarget(...)->写出 OutVT.POV->渲染用这个 POV 作为最终相机
	//可以把 UpdateViewTarget() 当成：相机每帧的“总装配函数”。
	if ((PendingViewTarget.Target != nullptr) && BlendParams.bLockOutgoing && OutVT.Equal(ViewTarget))
	{
		//这是 UE 相机切换时用的。比如从角色相机切到过场相机，有一个 blend 过程。bLockOutgoing 表示旧相机先锁住，不继续更新。如果正在做特殊相机混合，且旧镜头被锁住，就不要重新算这一帧。
		return;
	}
	//这一段是在把相机结果先清成默认值。因为 UpdateViewTarget() 每帧都会跑，如果不先初始化，有些上一帧遗留的数据可能影响这一帧。
	const FMinimalViewInfo OrigPOV = OutVT.POV;

	static const FMinimalViewInfo DefaultViewInfo;
	OutVT.POV = DefaultViewInfo;
	OutVT.POV.FOV = DefaultFOV;
	OutVT.POV.OrthoWidth = DefaultOrthoWidth;
	OutVT.POV.AspectRatio = DefaultAspectRatio;
	OutVT.POV.bConstrainAspectRatio = bDefaultConstrainAspectRatio;
	OutVT.POV.ProjectionMode = bIsOrthographic ? ECameraProjectionMode::Orthographic : ECameraProjectionMode::Perspective;
	OutVT.POV.PostProcessBlendWeight = 1.0f;
	OutVT.POV.bAutoCalculateOrthoPlanes = bAutoCalculateOrthoPlanes;
	OutVT.POV.AutoPlaneShift = AutoPlaneShift;
	OutVT.POV.bUpdateOrthoPlanes = bUpdateOrthoPlanes;
	OutVT.POV.bUseCameraHeightAsViewTarget = bUseCameraHeightAsViewTarget;

	bool bDoNotApplyModifiers = false;
	
	//这一段是在处理 UE 原生支持的几种相机模式。比如：
	//CameraActor：如果 ViewTarget 是一个 CameraActor，就直接用它自己的 CameraComponent。Fixed：固定不动。ThirdPerson / FreeCam：UE 默认第三人称/自由相机。FirstPerson：用角色眼睛位置。否则：走你的 Survivor 俯视相机。
	if (ACameraActor* CamActor = Cast<ACameraActor>(OutVT.Target))
	{
		CamActor->GetCameraComponent()->GetCameraView(DeltaTime, OutVT.POV);
	}
	else
	{
		//游戏正常战斗时不走 UE 那套通用 ThirdPerson/FirstPerson 逻辑，而是走 UpdateSurvivorViewTarget() 这个自定义俯视相机逻辑。除非你之后主动设置 CameraStyle = "ThirdPerson" 或者 SetViewTargetWithBlend 到某个 CameraActor，否则基本就是你的 Survivor 自定义逻辑。
		static const FName NAME_Fixed(TEXT("Fixed"));
		static const FName NAME_ThirdPerson(TEXT("ThirdPerson"));
		static const FName NAME_FreeCam(TEXT("FreeCam"));
		static const FName NAME_FreeCam_Default(TEXT("FreeCam_Default"));
		static const FName NAME_FirstPerson(TEXT("FirstPerson"));

		if (CameraStyle == NAME_Fixed)
		{
			OutVT.POV = OrigPOV;
			bDoNotApplyModifiers = true;
		}
		else if (CameraStyle == NAME_ThirdPerson || CameraStyle == NAME_FreeCam || CameraStyle == NAME_FreeCam_Default)
		{
			FVector Loc = OutVT.Target ? OutVT.Target->GetActorLocation() : FVector::ZeroVector;
			FRotator Rotator = OutVT.Target ? OutVT.Target->GetActorRotation() : FRotator::ZeroRotator;

			if (OutVT.Target == PCOwner && PCOwner)
			{
				Loc = PCOwner->GetFocalLocation();
			}

			if ((CameraStyle == NAME_FreeCam || CameraStyle == NAME_FreeCam_Default) && PCOwner)
			{
				Rotator = PCOwner->GetControlRotation();
			}

			OutVT.POV.Location = Loc + ViewTargetOffset + FRotationMatrix(Rotator).TransformVector(FreeCamOffset) - Rotator.Vector() * FreeCamDistance;
			OutVT.POV.Rotation = Rotator;
			bDoNotApplyModifiers = true;
		}
		else if (CameraStyle == NAME_FirstPerson)
		{
			if (OutVT.Target)
			{
				OutVT.Target->GetActorEyesViewPoint(OutVT.POV.Location, OutVT.POV.Rotation);
			}
			bDoNotApplyModifiers = true;
		}
		else
		{
			UpdateSurvivorViewTarget(OutVT, DeltaTime);
		}
	}
	//UE 的 StartCameraShake() 并不是立刻直接改屏幕。它通常会注册一个 camera modifier，然后每帧在这里影响 OutVT.POV。
	if (!bDoNotApplyModifiers || bAlwaysApplyModifiers)
	{
		//StartCameraShake(...) 创建出来的 UE 原生 CameraShake，通常就是通过这个系统影响相机的。
		ApplyCameraModifiers(DeltaTime, OutVT.POV); //UE 原生相机系统的入口。StartCameraShake() 之后，真正把 CameraShake 影响到镜头上的，通常就是这里。
	}
	//先有基础镜头（UpdateSurvivorViewTarget），再叠 UE 原生震屏（ApplyCameraModifiers），再叠你自己的保险震屏（ApplyFeedbackShakeFallback）。

	//它不依赖 UE CameraShake 是否生效，直接把最终镜头抖一下。
	ApplyFeedbackShakeFallback(DeltaTime, OutVT.POV); //哪怕 UE 原生 CameraShake 由于相机模式、asset 配置、play space 等原因不明显，你也能看到一个由 C++ 直接改最终 POV 的震屏效果。
	
	//这是把 PlayerCameraManager 自己的位置和旋转同步到最终 POV，并更新镜头特效。
	SetActorLocationAndRotation(OutVT.POV.Location, OutVT.POV.Rotation, false);
	if (bAutoCalculateOrthoPlanes && OutVT.Target)
	{
		OutVT.POV.SetCameraToViewTarget(OutVT.Target->GetActorLocation());
	}

	UpdateCameraLensEffects(OutVT);
}

void ASurvivorPlayerCameraManager::UpdateSurvivorViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	//基础镜头逻辑。类 Brotato/幸存者玩法常见的俯视跟随镜头：负责解算在正常情况下，镜头在哪里，看向哪里。
	const FVector TargetFocusLocation = ResolveCameraFocusLocation(OutVT.Target);
	if (!bHasSmoothedFocusLocation)
	{
		SmoothedFocusLocation = TargetFocusLocation;
		bHasSmoothedFocusLocation = true;
	}
	else
	{
		SmoothedFocusLocation = FMath::VInterpTo(
			SmoothedFocusLocation,
			TargetFocusLocation,
			DeltaTime,
			FMath::Max(0.0f, CameraFollowLagSpeed));
	}

	const FVector CameraDirection = CameraRotation.Vector();
	OutVT.POV.Location = SmoothedFocusLocation - (CameraDirection * CameraDistance);
	OutVT.POV.Rotation = CameraRotation;
	OutVT.POV.FOV = PerspectiveFOV;
}

void ASurvivorPlayerCameraManager::ApplyFeedbackShakeFallback(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	if (FeedbackShakeTimeRemaining <= 0.0f || FeedbackShakeDurationRemaining <= 0.0f || FeedbackShakeScale <= 0.0f)
	{
		return;
	}

	FeedbackShakeTimeRemaining = FMath::Max(0.0f, FeedbackShakeTimeRemaining - DeltaTime);
	const float Alpha = FeedbackShakeDurationRemaining > 0.0f
		? FMath::Clamp(FeedbackShakeTimeRemaining / FeedbackShakeDurationRemaining, 0.0f, 1.0f)
		: 0.0f;
	const float Envelope = Alpha * Alpha;
	if (Envelope <= 0.0f)
	{
		FeedbackShakeScale = 0.0f;
		return;
	}

	const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float LocWaveX = FMath::Sin((TimeSeconds * 70.0f) + FeedbackShakeLocationPhase);
	const float LocWaveY = FMath::Sin((TimeSeconds * 92.0f) + FeedbackShakeLocationPhase * 1.7f);
	const float RotWavePitch = FMath::Sin((TimeSeconds * 84.0f) + FeedbackShakeRotationPhase);
	const float RotWaveYaw = FMath::Cos((TimeSeconds * 63.0f) + FeedbackShakeRotationPhase * 1.3f);
	const float RotWaveRoll = FMath::Sin((TimeSeconds * 105.0f) + FeedbackShakeRotationPhase * 0.6f);

	const FVector RightVector = InOutPOV.Rotation.RotateVector(FVector::RightVector);
	const FVector UpVector = InOutPOV.Rotation.RotateVector(FVector::UpVector);
	const float LocationAmplitude = FeedbackShakeFallbackLocationAmplitude * FeedbackShakeScale * Envelope;
	const float RotationAmplitude = FeedbackShakeFallbackRotationAmplitude * FeedbackShakeScale * Envelope;

	InOutPOV.Location += (RightVector * LocWaveX * LocationAmplitude) + (UpVector * LocWaveY * LocationAmplitude);
	InOutPOV.Rotation.Pitch += RotWavePitch * RotationAmplitude;
	InOutPOV.Rotation.Yaw += RotWaveYaw * RotationAmplitude * 0.5f;
	InOutPOV.Rotation.Roll += RotWaveRoll * RotationAmplitude;

	if (FeedbackShakeTimeRemaining <= 0.0f)
	{
		FeedbackShakeScale = 0.0f;
	}
}

FVector ASurvivorPlayerCameraManager::ResolveCameraFocusLocation(const AActor* TargetActor) const
{
	if (const ASurvivorCharacter* SurvivorCharacter = ResolveSurvivorCharacter(TargetActor))
	{
		return SurvivorCharacter->GetCameraPivotLocation();
	}

	return TargetActor ? TargetActor->GetActorLocation() + CameraFocusOffset : CameraFocusOffset;
}

const ASurvivorCharacter* ASurvivorPlayerCameraManager::ResolveSurvivorCharacter(const AActor* TargetActor) const
{
	if (const ASurvivorCharacter* SurvivorCharacter = Cast<ASurvivorCharacter>(TargetActor))
	{
		return SurvivorCharacter;
	}

	const APlayerController* OwningController = GetOwningPlayerController();
	return OwningController ? Cast<ASurvivorCharacter>(OwningController->GetPawn()) : nullptr;
}
