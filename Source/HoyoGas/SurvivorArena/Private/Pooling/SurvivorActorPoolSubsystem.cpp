#include "Pooling/SurvivorActorPoolSubsystem.h"

#include "Core/SurvivorArenaLog.h"
#include "Pooling/SurvivorPooledActorInterface.h"

AActor* USurvivorActorPoolSubsystem::AcquireActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, AActor* NewOwner, APawn* NewInstigator)
{
	if (!ActorClass)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("ActorPool AcquireActor failed because ActorClass is null."));
		return nullptr;
	}

	AActor* Actor = nullptr;
	FSurvivorActorPoolBucket& Bucket = BucketsByClass.FindOrAdd(*ActorClass);
	while (!Bucket.AvailableActors.IsEmpty() && Actor == nullptr)
	{
		Actor = Bucket.AvailableActors.Pop();
	}

	if (Actor == nullptr)
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("ActorPool AcquireActor failed because World is unavailable for class %s."), *GetNameSafe(*ActorClass));
			return nullptr;
		}

		Actor = World->SpawnActor<AActor>(ActorClass, SpawnTransform);
		if (!Actor)
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("ActorPool AcquireActor failed to spawn actor of class %s."), *GetNameSafe(*ActorClass));
			return nullptr;
		}
	}

	PrepareActorForAcquire(Actor, SpawnTransform, NewOwner, NewInstigator);
	ActiveActorToClass.Add(Actor, *ActorClass);
	return Actor;
}

void USurvivorActorPoolSubsystem::ReleaseActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	const TWeakObjectPtr<UClass>* FoundClass = ActiveActorToClass.Find(Actor);
	UClass* ActorClass = FoundClass ? FoundClass->Get() : Actor->GetClass();
	ActiveActorToClass.Remove(Actor);

	if (!ActorClass)
	{
		Actor->Destroy();
		return;
	}

	PrepareActorForRelease(Actor);
	BucketsByClass.FindOrAdd(ActorClass).AvailableActors.Add(Actor);
}

void USurvivorActorPoolSubsystem::PrepareActorForAcquire(AActor* Actor, const FTransform& SpawnTransform, AActor* NewOwner, APawn* NewInstigator)
{
	if (!Actor)
	{
		return;
	}

	Actor->SetOwner(NewOwner);
	Actor->SetInstigator(NewInstigator);
	Actor->SetActorTransform(SpawnTransform);
	Actor->SetActorHiddenInGame(false);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorTickEnabled(true);

	if (Actor->GetClass()->ImplementsInterface(USurvivorPooledActorInterface::StaticClass()))
	{
		ISurvivorPooledActorInterface::Execute_OnAcquiredFromPool(Actor);
	}
}

void USurvivorActorPoolSubsystem::PrepareActorForRelease(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	if (Actor->GetClass()->ImplementsInterface(USurvivorPooledActorInterface::StaticClass()))
	{
		ISurvivorPooledActorInterface::Execute_OnReleasedToPool(Actor);
	}

	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);
	Actor->SetActorHiddenInGame(true);
	Actor->SetOwner(nullptr);
	Actor->SetInstigator(nullptr);
}
