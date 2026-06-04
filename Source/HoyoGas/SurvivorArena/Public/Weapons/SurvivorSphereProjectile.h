#pragma once

#include "CoreMinimal.h"
#include "Weapons/SurvivorProjectileBase.h"
#include "SurvivorSphereProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class HOYOGAS_API ASurvivorSphereProjectile : public ASurvivorProjectileBase
{
	GENERATED_BODY()

public:
	ASurvivorSphereProjectile();

	UFUNCTION(BlueprintPure, Category = "Survivor|Projectile")
	USphereComponent* GetSphereCollision() const;

	UFUNCTION(BlueprintPure, Category = "Survivor|Projectile")
	UStaticMeshComponent* GetVisualMesh() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> SphereCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> VisualMesh;
};
