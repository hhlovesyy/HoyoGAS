#pragma once

#include "CoreMinimal.h"
#include "Data/SurvivorArenaTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SurvivorRunLauncherSubsystem.generated.h"

UCLASS()
class HOYOGAS_API USurvivorRunLauncherSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SurvivorArena")
	bool LaunchSurvivorRun(const FSurvivorRunStartConfig& Config);

	bool ConsumePendingStartConfig(FSurvivorRunStartConfig& OutConfig);

private:
	FSurvivorRunStartConfig BuildResolvedConfig(const FSurvivorRunStartConfig& InConfig) const;

	UPROPERTY(Transient)
	FSurvivorRunStartConfig PendingStartConfig;

	bool bHasPendingStartConfig = false;
};
