#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "SurvivorCharacter.generated.h"

class UAbilitySystemComponent;
class UFaceShadowComponent;
class USceneComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class USurvivorAbilitySystemComponent;
class USurvivorPickupCollectorComponent;
class USurvivorWeaponManagerComponent;

UCLASS()
class HOYOGAS_API ASurvivorCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASurvivorCharacter();

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	USkeletalMeshComponent* GetSourceCharacterMesh() const;
	USkeletalMeshComponent* GetVisualCharacterMesh() const;
	USkeletalMeshComponent* GetWeaponMesh() const;
	USkeletalMeshComponent* GetOutlineMesh() const;
	UStaticMeshComponent* GetHeadAttachmentMesh() const;
	UFaceShadowComponent* GetFaceShadowComponent() const;
	USceneComponent* GetCameraPivot() const;
	FVector GetCameraPivotLocation() const;
	USurvivorWeaponManagerComponent* GetWeaponManagerComponent() const;
	USurvivorPickupCollectorComponent* GetPickupCollectorComponent() const;

protected:
	void InitAbilityActorInfo();
	void RefreshVisualComponentBindings();
	USkeletalMeshComponent* ResolvePrimaryVisualMesh() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> CameraPivot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Camera", meta = (AllowPrivateAccess = "true"))
	FVector CameraPivotRelativeLocation = FVector(0.0f, 0.0f, 120.0f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> VisualCharacterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> HeadAttachmentMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> OutlineMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFaceShadowComponent> FaceShadowComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Weapons", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USurvivorWeaponManagerComponent> WeaponManagerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Pickup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USurvivorPickupCollectorComponent> PickupCollectorComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Visual", meta = (AllowPrivateAccess = "true"))
	FName HeadAttachmentSocketName = TEXT("HeadDecalSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Visual", meta = (AllowPrivateAccess = "true"))
	FName WeaponAttachSocketName = TEXT("WeaponHandSocket");

	UPROPERTY(Transient)
	TObjectPtr<USurvivorAbilitySystemComponent> CachedAbilitySystemComponent;
};
