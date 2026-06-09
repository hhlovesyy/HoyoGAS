#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SurvivorActorPoolSubsystem.generated.h"

class APawn;

USTRUCT()
struct FSurvivorActorPoolBucket
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> AvailableActors;
};

UCLASS()
class HOYOGAS_API USurvivorActorPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	AActor* AcquireActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, AActor* NewOwner = nullptr, APawn* NewInstigator = nullptr);
	void ReleaseActor(AActor* Actor);

protected:
	void PrepareActorForAcquire(AActor* Actor, const FTransform& SpawnTransform, AActor* NewOwner, APawn* NewInstigator);
	void PrepareActorForRelease(AActor* Actor);

	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, FSurvivorActorPoolBucket> BucketsByClass;

	TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<UClass>> ActiveActorToClass;
};
