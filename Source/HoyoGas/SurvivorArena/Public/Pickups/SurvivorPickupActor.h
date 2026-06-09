#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pooling/SurvivorPooledActorInterface.h"
#include "SurvivorPickupActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class USceneComponent;
class UPrimitiveComponent;
class USurvivorPickupDefinition;
class USurvivorRunEconomyComponent;

UCLASS()
class HOYOGAS_API ASurvivorPickupActor : public AActor, public ISurvivorPooledActorInterface
{
	GENERATED_BODY()

public:
	ASurvivorPickupActor();

	virtual void Tick(float DeltaSeconds) override;

	void InitializePickup(USurvivorPickupDefinition* InPickupDefinition, int32 InGrantedValue);
	void BeginMagnetTowards(AActor* CollectorActor);
	bool TryCollect(AActor* CollectorActor);

	virtual void OnAcquiredFromPool_Implementation() override;
	virtual void OnReleasedToPool_Implementation() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandlePickupOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	USurvivorRunEconomyComponent* ResolveEconomyComponent(AActor* CollectorActor) const;
	void ReleaseToPoolOrDestroy();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Pickup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Pickup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Pickup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(Transient)
	TObjectPtr<USurvivorPickupDefinition> PickupDefinition;

	UPROPERTY(Transient)
	int32 GrantedValue = 0;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> MagnetTargetActor;

	UPROPERTY(Transient)
	bool bCollected = false;

	UPROPERTY(Transient)
	bool bWasAcquiredFromPool = false;
};
