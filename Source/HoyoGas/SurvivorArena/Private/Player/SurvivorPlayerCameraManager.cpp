#include "Player/SurvivorPlayerCameraManager.h"

#include "Core/SurvivorArenaLog.h"
#include "GameFramework/PlayerController.h"
#include "Player/SurvivorCharacter.h"

/*
 *在 UE5 中，很多新手习惯把摄像机（Camera Component）和弹簧臂（Spring Arm Component）直接挂在角色蓝图（Character）身上。
 *但在这个类中，开发者完全剥离了摄像机与角色的强绑定，将摄像机的控制权交给了全局的 PlayerCameraManager。这是一种非常成熟的解耦设计。
1. 核心三角关系
在 UE5 中，角色、输入和镜头是严格分离的：

Character (演员)： 只负责移动逻辑、播放动画、放技能。它根本不知道摄像机的存在。

PlayerCameraManager (摄影师)： 只负责算位置、算角度（就是咱们上一趴看的代码）。

PlayerController (导演)： 它是玩家意志的具象化。它既拥有“摄影师”（CameraManager 是它的组件），又能控制“演员”。

2. ViewTarget 是怎么自动赋值的？
你不写任何一行专门绑定摄像机的代码，引擎是这样自动跑通的：

附身 (Possess)： 游戏开始时，GameMode 安排你的 PlayerController 去接管主角，这个动作在 UE 里叫 Possess（附身）。

设为焦点 (Set ViewTarget)： 在 Possess 成功的瞬间，PlayerController 的底层 C++ 会触发一个默认行为——自动把被附身的 Character 设为当前的 ViewTarget（观察目标）。

摄影师工作： PlayerCameraManager 每一帧都会向老板（PlayerController）要数据。它拿到这个 ViewTarget 后，就会作为参数传入 UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) 函数中（也就是你给的代码里的 OutVT.Target）。 
*/
ASurvivorPlayerCameraManager::ASurvivorPlayerCameraManager()
{
	ViewPitchMin = -89.0f;
	ViewPitchMax = 0.0f;
	DefaultFOV = PerspectiveFOV;
}

void ASurvivorPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	/*
	*找目标 (Resolve Target)： 首先确定摄像机当前该盯着谁（通常是主角）。

	平滑移动 (VInterpTo)： 这是这个游戏摄像机手感的灵魂。它没有让摄像机死板地瞬间移动到主角头顶，而是使用 FMath::VInterpTo 做了插值平滑处理。

	SmoothedFocusLocation 记录了摄像机当前的“虚拟焦点”。

	每一帧，这个焦点都会根据 CameraFollowLagSpeed（追赶速度，当前默认是 8.0f）逐渐向主角的真实位置靠拢。这就形成了类似于“弹簧臂滞后（Camera Lag）”的柔和跟随效果。

	计算最终机位：

	CameraDirection = CameraRotation.Vector();：获取摄像机朝向（由固定设定的 -60 度俯角决定）。

	SmoothedFocusLocation - (CameraDirection * CameraDistance);：重点在这里。它通过向量数学，从平滑后的焦点位置，沿着摄像机的视线方向反向倒退了 CameraDistance（1400.0f）的距离，从而计算出摄像机最终应该悬挂在半空中的三维坐标。
	 */
	Super::UpdateViewTarget(OutVT, DeltaTime);

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
	const FVector CameraLocation = SmoothedFocusLocation - (CameraDirection * CameraDistance);

	OutVT.POV.Location = CameraLocation;
	OutVT.POV.Rotation = CameraRotation;
	OutVT.POV.FOV = PerspectiveFOV;
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
