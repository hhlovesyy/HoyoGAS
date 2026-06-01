#pragma once

#include "CoreMinimal.h"
#include "Data/SurvivorArenaTypes.h"
#include "GameFramework/GameModeBase.h"
#include "SurvivorGameMode.generated.h"

UCLASS()
class HOYOGAS_API ASurvivorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASurvivorGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "SurvivorArena")
	void StartSurvivorRun(const FSurvivorRunStartConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "SurvivorArena")
	void EndSurvivorRun(bool bVictory);

protected:
	void SetRunState(ESurvivorRunState NewRunState);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	ESurvivorRunState CurrentRunState = ESurvivorRunState::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FSurvivorRunStartConfig CurrentRunConfig;

	float RunStartTimeSeconds = 0.0f;
};
