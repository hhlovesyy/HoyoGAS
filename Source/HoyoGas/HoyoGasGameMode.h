// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HoyoGasGameMode.generated.h"

UCLASS(minimalapi)
class AHoyoGasGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHoyoGasGameMode();
	virtual void StartPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;

private:
	bool SpawnGameplayDemoPickups(APawn* PlayerPawn);

	bool bGameplayDemoPickupsSpawned = false;
};



