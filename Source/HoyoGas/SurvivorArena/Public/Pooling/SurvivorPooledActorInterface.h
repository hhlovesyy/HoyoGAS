#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SurvivorPooledActorInterface.generated.h"

UINTERFACE(BlueprintType)
class HOYOGAS_API USurvivorPooledActorInterface : public UInterface
{
	GENERATED_BODY()
};

class HOYOGAS_API ISurvivorPooledActorInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Survivor|Pooling")
	void OnAcquiredFromPool();

	UFUNCTION(BlueprintNativeEvent, Category = "Survivor|Pooling")
	void OnReleasedToPool();
};
