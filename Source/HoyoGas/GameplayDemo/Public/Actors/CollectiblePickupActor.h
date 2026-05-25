#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CollectiblePickupActor.generated.h"

class UBillboardComponent;
class USphereComponent;
class UStaticMesh;
class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class HOYOGAS_API ACollectiblePickupActor : public AActor
{
	GENERATED_BODY()

public:
	ACollectiblePickupActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo")
	FName ItemId = TEXT("RedGem");

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandlePickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void RefreshVisuals();

	UPROPERTY(VisibleAnywhere, Category = "GameplayDemo")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "GameplayDemo")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "GameplayDemo")
	TObjectPtr<UBillboardComponent> BillboardComponent;

	UPROPERTY(VisibleAnywhere, Category = "GameplayDemo")
	TObjectPtr<USphereComponent> PickupCollision;

	UPROPERTY(EditAnywhere, Category = "GameplayDemo")
	float SpinSpeedDegreesPerSecond = 60.0f;

	UPROPERTY()
	TObjectPtr<UStaticMesh> SphereMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> ConeMesh;
};
