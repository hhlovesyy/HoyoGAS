#include "Weapons/SurvivorSphereProjectile.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

ASurvivorSphereProjectile::ASurvivorSphereProjectile()
{
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetupAttachment(SceneRoot);
	SphereCollision->InitSphereRadius(24.0f);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetGenerateOverlapEvents(true);
	SphereCollision->SetCollisionObjectType(ECC_WorldDynamic);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Overlap);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(SphereCollision);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetPrimaryCollisionComponent(SphereCollision);
}

USphereComponent* ASurvivorSphereProjectile::GetSphereCollision() const
{
	return SphereCollision;
}

UStaticMeshComponent* ASurvivorSphereProjectile::GetVisualMesh() const
{
	return VisualMesh;
}
