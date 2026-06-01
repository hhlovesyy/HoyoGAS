#include "Interaction/SurvivorArenaEntranceActor.h"

#include "Components/SceneComponent.h"
#include "Core/SurvivorArenaLog.h"
#include "Core/SurvivorRunLauncherSubsystem.h"
#include "GameFramework/PlayerController.h"

ASurvivorArenaEntranceActor::ASurvivorArenaEntranceActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

bool ASurvivorArenaEntranceActor::CanInteract_Implementation(APlayerController* InteractingController) const
{
	return true;
}

FText ASurvivorArenaEntranceActor::GetInteractionText_Implementation() const
{
	return InteractionText;
}

void ASurvivorArenaEntranceActor::Interact_Implementation(APlayerController* InteractingController)
{
	UGameInstance* GameInstance = InteractingController ? InteractingController->GetGameInstance() : GetGameInstance();
	USurvivorRunLauncherSubsystem* LauncherSubsystem = GameInstance ? GameInstance->GetSubsystem<USurvivorRunLauncherSubsystem>() : nullptr;
	if (!LauncherSubsystem)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("SurvivorArenaEntranceActor %s could not launch run because launcher subsystem is unavailable."), *GetNameSafe(this));
		return;
	}

	UE_LOG(LogSurvivorArena, Log, TEXT("SurvivorArena entrance interacted: %s"), *GetNameSafe(this));
	LauncherSubsystem->LaunchSurvivorRun(StartConfigOverride);
}
